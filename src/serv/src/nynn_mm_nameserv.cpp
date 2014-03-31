#include<zmq.hpp>
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_handler.hpp>
#include<nynn_mm_graph_table.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;

static pthread_key_t flag_key;
unique_ptr<GraphTable> graphtable;
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
void* switcher(void*args){
	X& x=*(X*)args;
	zmq::proxy((void*)*x.frontend,(void*)*x.backend,NULL);
	return NULL;
}


void* worker(void*args){
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
	//initialize datasocks
	ZMQSockMap datasocks;
	istringstream iss(getenv("NYNN_MM_DATASERV_LIST"));
	vector<string> hosts=get_a_line_of_words(iss);
	string localhost=get_host();
	uint32_t data_port=parse_int(getenv("NYNN_MM_DATASERV_PORT"),40001);
	for (int i=0;i<hosts.size();i++){
		uint32_t ip=host2ip(hosts[i]);
		string data_endpoint="tcp://"+ip2string(ip)+":"+to_string(data_port);
		datasocks[ip].reset(new zmq::socket_t(ctx,ZMQ_REQ));
		datasocks[ip]->connect(data_endpoint.c_str());
	}

	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	uint32_t replics_num=parse_int(getenv("NYNN_MM_REPLICAS_NUM"),3);
	while(flag){
		zmq::poll(items.get(),socketNum,-1);
		for (int i=0;i<socketNum;i++){
			if (items[i].revents&ZMQ_POLLIN){
				prot::Replier rep(*sockets[i].get());
				rep.parse_ask();
				switch(rep.get_cmd()){
				case prot::CMD_SUBMIT:
					handle_submit(rep,*graphtable.get());
					break;
				case prot::CMD_WRITE:
					handle_write(rep,*graphtable.get(),datasocks);
					break;
				case prot::CMD_HELLO:
					handle_hello(rep,*graphtable.get());
					break;
				default:
					break;
				}
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

	//initialization
	uint32_t replica=parse_int(getenv("NYNN_MM_REPLICAS"),3);
	graphtable.reset(new GraphTable(replica));


	add_signal_handler(SIGTERM,&kill_thread);
	add_signal_handler(SIGINT,SIG_IGN);
	thread_key_t create(&flag_key,NULL);
	
	//creating switcher_thd,logger,worker_thds threads.
	zmq::context_t ctx;//ctx(io_threads)
	zmq::socket_t collector(ctx,ZMQ_ROUTER);
	zmq::socket_t dispatcher(ctx,ZMQ_DEALER);
	
	uint32_t port=parse_int(getenv("NYNN_MM_NAMESERV_PORT"),40002);
	string collector_endpoint=string("tcp://")+ ip2string(get_ip())+ ":"+to_string(port); 
	log_i("collector endpoint: %s",collector_endpoint.c_str());

	//create switcher_thd
	collector.bind(collector_endpoint.c_str());
	dispatcher.bind("inproc://dispatcher.inproc");
	X x={&collector,&dispatcher};
	thread_t switcher_thd(switcher,&x);
	switcher_thd.start();

	//create worker_thds
	uint32_t work_thd_num=parse_int(getenv("NYNN_MM_NAMESERV_WORKER_NUM"),3);
	unique_ptr<thread_t> *worker_thds=new unique_ptr<thread_t>[work_thd_num];
	for (int i=0;i<work_thd_num;i++)worker_thds[i].reset(new thread_t(worker,&ctx));
	for (int i=0;i<work_thd_num;i++)worker_thds[i]->start();

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGQUIT);
	cout<<"wait for 'SIGQUIT' to terminate process"<<endl;
	int signum;
	sigwait(&sigs,&signum);
	cout<<"process terminated by 'SIGQUIT'"<<endl;

	//shutdown all worker_thds gracefully.
	for (int i=0;i<work_thd_num;i++){
		nanosleep_for(5000);
		if (worker_thds[i]->is_alive())worker_thds[i]->kill(SIGTERM);
	}
	nanosleep_for(5000);

	for (int i=0;i<work_thd_num;i++){worker_thds[i]->join();}
	log_i("all worker_thds are shutdown");
	
	//shutdown switcher_thd gracefully.
	collector.close();
	dispatcher.close();
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.kill(SIGTERM);
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.stop();
	switcher_thd.join();
	log_i("switcher_thd is shutdown");
}
