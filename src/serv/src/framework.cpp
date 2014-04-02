#include<zmq.hpp>
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_graph.hpp>
#include<nynn_mm_handler.hpp>

using namespace std;
using namespace nynn;

static pthread_key_t flag_key;

class thread_key_t{
public:
	thread_key_t(pthread_key_t* k,void(*destructor)(void*)):key(k){
		pthread_key_create(key,destructor);
	}
	~thread_key_t(){pthread_key_delete(*key);}
private:
	pthread_key_t * key;
};

void kill_thread(int signum){
	pthread_setspecific(flag_key,(void*)0);
}

typedef struct{
	zmq::socket_t *frontend;
	zmq::socket_t *backend;
}X;
void* switcher(void*args){
	X& x=*(X*)args;
	zmq::proxy((void*)*x.frontend,(void*)*x.backend,NULL);
	return NULL;
}

void* worker(void*args)
{
	try
	{
	zmq::context_t& ctx=*(zmq::context_t*)args;
	int socket_num=parse_int(getenv("NYNN_MM_DATASERV_SOCKET_NUM_PER_WORKER"),10);
	unique_ptr<unique_ptr<zmq::socket_t>[]> sockets;
	sockets.reset(new unique_ptr<zmq::socket_t>[socket_num]);
	unique_ptr<zmq::pollitem_t[]> items;
	items.reset(new zmq::pollitem_t[socket_num]);

	for (int i=0;i<socket_num;i++){
		sockets[i].reset(new zmq::socket_t(ctx,ZMQ_REP));
		sockets[i]->connect("inproc://dispatcher.inproc");
		items[i].socket=(void*)*sockets[i].get();
		items[i].events=ZMQ_POLLIN|ZMQ_POLLERR;
	}
	
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	while(flag){
		zmq::poll(items.get(),socket_num,-1);
		for (int i=0;i<socket_num;i++){
			if (items[i].revents&ZMQ_POLLIN){
				prot::Replier rep(*sockets[i].get());
				rep.parse_ask();
				char data[1<<16];
				rep.ans(prot::STATUS_OK,data,1<<16);
			}
			if (items[i].revents&ZMQ_POLLERR){
				pthread_setspecific(flag_key,(void*)0);
				break;
			}
		}
		flag=(intptr_t)pthread_getspecific(flag_key);
	}
	}catch(zmq::error_t& err){
		log_w(err.what());
	}
	log_i("work terminated normally");
}

int main(){

	add_signal_handler(SIGTERM,&kill_thread);
	add_signal_handler(SIGINT,SIG_IGN);
	add_signal_handler(SIGABRT,SIG_IGN);
	thread_key_t create(&flag_key,NULL);
	//creating switcher,logger,worker_thds threads.
	zmq::context_t ctx;//ctx(io_threads)
	zmq_ctx_set(ctx,ZMQ_IO_THREADS,10);
	zmq_ctx_set(ctx,ZMQ_MAX_SOCKETS,4096);
	zmq::socket_t collector(ctx,ZMQ_ROUTER);
	zmq::socket_t dispatcher(ctx,ZMQ_DEALER);
	
	uint32_t port=parse_int(getenv("NYNN_MM_DATASERV_PORT"),40001);
	string collector_endpoint=string("tcp://")+ ip2string(get_ip())+ ":"+to_string(port); 
	string collector_endpoint_local=string("ipc://")+getenv("NYNN_MM_DATASERV_IPC");
	log_i("collector endpoint: %s",collector_endpoint.c_str());

	//create switcher
	collector.bind(collector_endpoint.c_str());
	collector.bind(collector_endpoint_local.c_str());
	dispatcher.bind("inproc://dispatcher.inproc");
	X x={&collector,&dispatcher};
	thread_t switcher_thd(switcher,&x);
	switcher_thd.start();

	//create worker_thds
	uint32_t workerNum=parse_int(getenv("NYNN_MM_DATASERV_WORKER_NUM"),3);
	unique_ptr<thread_t> *worker_thds=new unique_ptr<thread_t>[workerNum];
	for (int i=0;i<workerNum;i++)worker_thds[i].reset(new thread_t(worker,&ctx));
	for (int i=0;i<workerNum;i++)worker_thds[i]->start();


	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGQUIT);
	cout<<"wait for 'SIGQUIT' to terminate process"<<endl;
	int signum;
	sigwait(&sigs,&signum);
	cout<<"process terminated by 'SIGQUIT'"<<endl;

	//shutdown all worker_thds gracefully.
	for (int i=0;i<workerNum;i++){
		nanosleep_for(5000);
		if (worker_thds[i]->is_alive())worker_thds[i]->kill(SIGTERM);
	}
	nanosleep_for(5000);
	/*
	for (int i=0;i<workerNum;i++){
		if (worker_thds[i]->is_alive())worker_thds[i]->stop();
	}
	*/
	for (int i=0;i<workerNum;i++){worker_thds[i]->join();}
	log_i("all worker_thds are shutdown");
	
	//shutdown switcher gracefully.
	collector.close();
	dispatcher.close();
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.kill(SIGTERM);
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.stop();
	switcher_thd.join();
	log_i("switcher is shutdown");
}
