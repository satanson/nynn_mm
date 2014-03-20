#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<zmq.hpp>
#include<nynn_mm_graph_table.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;

#define VERSION_NO 0X02000000

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

enum{VERSION_PART,CMD_PART,OPTIONS_PART,DATA_PART,PART_NUM};

void* do_work(void*args){
	zmq::context_t& ctx=*(zmq::context_t*)args;
	int socketNum=parse_int(getenv("NYNN_MM_NAMESERV_SOCKET_NUM_PER_WORKER"),10);
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
	zmq::message_t msg[PART_NUM];
	while(flag){
		zmq::poll(items.get(),socketNum,-1);
		for (int i=0;i<socketNum;i++){
			if (items[i].revents&ZMQ_POLLIN){
				int k=0;
				do{
					sockets[i]->recv(msg[k],0);
				}while(msg[k++].more()&&k<PART_NUM);
				
				if (k!=PART_NUM){
					log_w("require %d parts,but actually recv %d part(s)",PART_NUM,k);
					continue;
				}
				handle_msg();
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

unique_ptr<GraphTable> graphTable;
unordered_map<string,unique_ptr<zmq::socket_t> > dataNodes;

int main(){

	//initialization
	uint32_t replica=parse_int(getenv("NYNN_MM_REPLICAS"),3);
	graphTable.reset(new GraphTable(replica));


	add_signal_handler(SIGTERM,&kill_thread);
	add_signal_handler(SIGINT,SIG_IGN);
	thread_key_t create(&flag_key,NULL);
	
	//creating switcher,logger,workers threads.
	zmq::context_t ctx;//ctx(io_threads)
	zmq::socket_t collector(ctx,ZMQ_ROUTER);
	zmq::socket_t dispatcher(ctx,ZMQ_DEALER);
	
	uint32_t port=parse_int(getenv("NYNN_MM_NAMESERV_PORT"),40002);
	string collector_endpoint=string("tcp://")+ ip2string(get_ip())+ ":"+to_string(port); 
	log_i("collector endpoint: %s",collector_endpoint.c_str());

	//create switcher
	collector.bind(collector_endpoint.c_str());
	dispatcher.bind("inproc://dispatcher.inproc");
	X x={&collector,&dispatcher};
	thread_t switcher(do_switch,&x);
	switcher.start();

	//create workers
	uint32_t workerNum=parse_int(getenv("NYNN_MM_NAMESERV_WORKER_NUM"),3);
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
