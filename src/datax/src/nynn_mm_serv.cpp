#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<zmq.hpp>

using namespace std;
using namespace nynn;

static pthread_key_t flag_key;
void* func(void*args){
	int i=(intptr_t)args;
	
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	cout<<"thread "<<i<<" is created"<<endl;
	while(flag){
		sleep(1);
		cout<<"thread "<<i<<" is runnning...."<<endl;
		flag=(intptr_t)pthread_getspecific(flag_key);
	}
	cout<<"thread "<<i<<"is terminated"<<endl;
}
void kill_thread(int signum){
	pthread_setspecific(flag_key,(void*)0);
}
class thread_key_t{
public:
	thread_key_t(pthread_key_t* k,void(*destructor)(void*)):key(k){
		pthread_key_create(key,destructor);
	}
	~thread_key_t(){pthread_key_delete(*key);}
private:
	pthread_key_t * key;
};


typedef struct{
	zmq::socket_t *frontend;
	zmq::socket_t *backend;
}X;
void* do_switch(void*args){
	X& x=*(X*)args;
	zmq::proxy((void*)*x.frontend,(void*)*x.backend,NULL);
	return NULL;
}

void* do_work(void*args){
	zmq::context_t& ctx=*(zmq::context_t*)args;
	int socketNum=parse_int(getenv("NYNN_MM_SERV_SOCKET_NUM_PER_WORKER"),10);
	unique_ptr<unique_ptr<zmq::socket_t>[]> sockets;
	sockets.reset(new unique_ptr<zmq::socket_t>[socketNum]);
	unique_ptr<zmq::pollitem_t[]> items;
	items.reset(new zmq::pollitem_t[socketNum]);

	for (int i=0;i<socketNum;i++){
		sockets[i].reset(new zmq::socket_t(ctx,ZMQ_REP));
		sockets[i]->connect("inproc://dispatcher.inproc");
		items[i].socket=(void*)*sockets[i].get();
		items[i].events=ZMQ_POLLIN|ZMQ_POLLERR;
	}

	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	zmq::message_t msg;
	while(flag){
		zmq::poll(items.get(),socketNum,-1);
		for (int i=0;i<socketNum;i++){
			if (items[i].revents&ZMQ_POLLIN){
				msg.rebuild();
				sockets[i]->recv(msg,0);
				string req(msg.size(),0);
				//processing
				memcpy(&req[0],msg.data(),msg.size());
				cout<<"RECV:"<<req<<endl;
				for (int k=0;k<req.size();k++)req[k]=toupper(req[k]);
				
				msg.rebuild(req.size());
				memcpy(msg.data(),req.data(),req.size());
				sockets[i]->send(msg,ZMQ_DONTWAIT);
			}
			if (items[i].revents&ZMQ_POLLERR){
				pthread_setspecific(flag_key,(void*)0);
				break;
			}
		}
		
		flag=(intptr_t)pthread_getspecific(flag_key);
	}
	log_i("work terminated normally");
}
int main(){

	add_signal_handler(SIGTERM,&kill_thread);
	add_signal_handler(SIGINT,SIG_IGN);
	thread_key_t create(&flag_key,NULL);
	
	//creating switcher,logger,workers threads.
	zmq::context_t ctx;//ctx(io_threads)
	zmq::socket_t collector(ctx,ZMQ_ROUTER);
	zmq::socket_t dispatcher(ctx,ZMQ_DEALER);
	
	uint32_t port=parse_int(getenv("NYNN_MM_SERV_PORT"),40001);
	string collector_endpoint=string("tcp://")+ ip2string(get_ip())+ ":"+to_string(port); 
	log_i("collector endpoint: %s",collector_endpoint.c_str());

	//create switcher
	collector.bind(collector_endpoint.c_str());
	dispatcher.bind("inproc://dispatcher.inproc");
	X x={&collector,&dispatcher};
	thread_t switcher(do_switch,&x);
	switcher.start();

	//create workers
	uint32_t workerNum=parse_int(getenv("NYNN_MM_SERV_WORER_NUM"),3);
	unique_ptr<thread_t> *workers=new unique_ptr<thread_t>[workerNum];
	for (int i=0;i<workerNum;i++)workers[i].reset(new thread_t(do_work,&ctx));
	for (int i=0;i<workerNum;i++)workers[i]->start();

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGQUIT);
	cout<<"wait for 'SIGQUIT' to terminate process"<<endl;
	int signum;
	sigwait(&sigs,&signum);
	cout<<"process terminated by 'SIGQUIT'"<<endl;

	//shutdown all workers gracefully.
	for (int i=0;i<workerNum;i++){
		nanosleep_for(5000);
		if (workers[i]->is_alive())workers[i]->kill(SIGTERM);
	}
	nanosleep_for(5000);
	/*
	for (int i=0;i<workerNum;i++){
		if (workers[i]->is_alive())workers[i]->stop();
	}
	*/
	for (int i=0;i<workerNum;i++){workers[i]->join();}
	log_i("all workers are shutdown");
	
	//shutdown switcher gracefully.
	collector.close();
	dispatcher.close();
	nanosleep_for(5000);
	if (switcher.is_alive())switcher.kill(SIGTERM);
	nanosleep_for(5000);
	if (switcher.is_alive())switcher.stop();
	switcher.join();
	log_i("switcher is shutdown");
}
