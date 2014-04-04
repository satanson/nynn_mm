#include<zmq.hpp>
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_graph.hpp>
#include<nynn_mm_handler.hpp>

using namespace std;
using namespace nynn;

typedef unique_ptr<zmq::socket_t> ZMQSock;
typedef unique_ptr<ZMQSock[]> ZMQSockArray;
typedef unique_ptr<unique_ptr<thread_t>[]> ThreadArray;

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
	ZMQSockArray sockets(new ZMQSock[socket_num]);
	unique_ptr<zmq::pollitem_t[]> items(new zmq::pollitem_t[socket_num]);

	uint32_t port_range_min=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MIN"),50000);
	uint32_t port_range_max=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MAX"),50010);
	for (int i=0;i<socket_num;i++){
		sockets[i].reset(new zmq::socket_t(ctx,ZMQ_REP));
		string suffix=to_string(rand_range(port_range_min,port_range_max));
		string endpoint=("inproc://scatter.")+suffix;
		log_i("worker:%s",endpoint.c_str());
		sockets[i]->connect(endpoint.c_str());
		items[i].socket=(void*)*sockets[i].get();
		items[i].events=ZMQ_POLLIN|ZMQ_POLLERR;
	}
	
	uint32_t MMU=parse_int(getenv("NYNN_MM_DATASERV_MMU"),512); 
	vector<char> data(MMU,0);
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	while(flag){
		zmq::poll(items.get(),socket_num,-1);
		for (int i=0;i<socket_num;i++){
			if (items[i].revents&ZMQ_POLLIN){
				prot::Replier rep(*sockets[i].get());
				rep.parse_ask();
				rep.ans(prot::STATUS_OK,&data[0],data.size());
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
	zmq_ctx_set(ctx,ZMQ_IO_THREADS,16);
	zmq_ctx_set(ctx,ZMQ_MAX_SOCKETS,4096);

	uint32_t port_range_min=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MIN"),50000);
	uint32_t port_range_max=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MAX"),50010);
	uint32_t port_range_num=port_range_max-port_range_min;
	uint32_t hwm=parse_int(getenv("NYNN_MM_DATASERV_HWM"),2000);
	uint32_t buffsz=parse_int(getenv("NYNN_MM_DATASERV_BUFFSZ"),1<<16);
	uint64_t affinity=parse_int(getenv("NYNN_MM_DATASERV_AFFINITY"),0);
	
	ZMQSockArray gathers(new ZMQSock[port_range_min]);
	ZMQSockArray scatters(new ZMQSock[port_range_min]);
	ThreadArray switcher_thds(new unique_ptr<thread_t>[port_range_num]);

	for( uint32_t i=0;i<port_range_num;i++){
		uint32_t port=port_range_min+i;
		string gather_endpoint=string("tcp://")+ip2string(get_ip())+":"+to_string(port);
		string scatter_endpoint=string("inproc://scatter.")+to_string(port);
		log_i("switcher%d:gather:%s",i,gather_endpoint.c_str());
		log_i("switcher%d:scatter:%s",i,scatter_endpoint.c_str());
		gathers[i].reset(new zmq::socket_t(ctx,ZMQ_ROUTER));
		gathers[i]->bind(gather_endpoint.c_str());
		gathers[i]->setsockopt(ZMQ_SNDHWM,&hwm,sizeof(hwm));
		gathers[i]->setsockopt(ZMQ_RCVHWM,&hwm,sizeof(hwm));
		gathers[i]->setsockopt(ZMQ_SNDBUF,&buffsz,sizeof(buffsz));
		gathers[i]->setsockopt(ZMQ_RCVBUF,&buffsz,sizeof(buffsz));
		gathers[i]->setsockopt(ZMQ_AFFINITY,&affinity,sizeof(affinity));
		
		scatters[i].reset(new zmq::socket_t(ctx,ZMQ_DEALER));
		scatters[i]->bind(scatter_endpoint.c_str());
		scatters[i]->setsockopt(ZMQ_SNDHWM,&hwm,sizeof(hwm));
		scatters[i]->setsockopt(ZMQ_RCVHWM,&hwm,sizeof(hwm));
		scatters[i]->setsockopt(ZMQ_SNDBUF,&buffsz,sizeof(buffsz));
		scatters[i]->setsockopt(ZMQ_RCVBUF,&buffsz,sizeof(buffsz));
		scatters[i]->setsockopt(ZMQ_AFFINITY,&affinity,sizeof(affinity));
		X x={gathers[i].get(),scatters[i].get()};
		switcher_thds[i].reset(new thread_t(switcher,&x));
		switcher_thds[i]->start();
	}
	log_i("create %d switcher for serv port from %d to %d",port_range_num,port_range_min,port_range_max);

	//create worker_thds
	uint32_t worker_num=parse_int(getenv("NYNN_MM_DATASERV_WORKER_NUM"),3);
	ThreadArray worker_thds(new unique_ptr<thread_t>[worker_num]);
	for (int i=0;i<worker_num;i++)worker_thds[i].reset(new thread_t(worker,&ctx));
	for (int i=0;i<worker_num;i++)worker_thds[i]->start();


	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGQUIT);
	cout<<"wait for 'SIGQUIT' to terminate process"<<endl;
	int signum;
	sigwait(&sigs,&signum);
	cout<<"process terminated by 'SIGQUIT'"<<endl;

	//shutdown all worker_thds gracefully.
	for (int i=0;i<worker_num;i++){
		nanosleep_for(5000);
		if (worker_thds[i]->is_alive())worker_thds[i]->kill(SIGTERM);
	}
	nanosleep_for(5000);
	/*
	for (int i=0;i<worker_num;i++){
		if (worker_thds[i]->is_alive())worker_thds[i]->stop();
	}
	*/
	for (int i=0;i<worker_num;i++){worker_thds[i]->join();}
	log_i("all workers are shutdown");
	
	//shutdown switcher gracefully.
	for (int i=0;i<port_range_num;i++){
		gathers[i]->close();
		scatters[i]->close();
	}
	nanosleep_for(5000);
	for (int i=0;i<port_range_num;i++){
		if (switcher_thds[i]->is_alive())switcher_thds[i]->kill(SIGTERM);
	}
	nanosleep_for(5000);
	for (int i=0;i<port_range_num;i++){
		if (switcher_thds[i]->is_alive())switcher_thds[i]->stop();
		switcher_thds[i]->join();
	}
	log_i("are switchers are shutdown");
}
