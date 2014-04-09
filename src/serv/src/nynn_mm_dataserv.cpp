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

static unique_ptr<Graph> graph;
static RWLock glock;
static pthread_t talkerid;

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

	uint32_t port_range_min=parse_int(getenv("NYNN_MM_NAMESERV_PORT_RANGE_MIN"),60000);
	uint32_t port_range_max=parse_int(getenv("NYNN_MM_NAMESERV_PORT_RANGE_MAX"),60008);
	for (int i=0;i<socket_num;i++){
		sockets[i].reset(new zmq::socket_t(ctx,ZMQ_REP));
		string suffix=to_string(rand_range(port_range_min,port_range_max));
		string endpoint=("inproc://scatter.")+suffix;
		log_i("worker:%s",endpoint.c_str());
		sockets[i]->connect(endpoint.c_str());
		items[i].socket=(void*)*sockets[i].get();
		items[i].events=ZMQ_POLLIN|ZMQ_POLLERR;
	}
	
	//initialize datasocks
	ZMQSockMap datasocks;
	istringstream iss(getenv("NYNN_MM_DATASERV_HOST_LIST"));
	vector<string> hosts=get_a_line_of_words(iss);
	string localhost=get_host();
	//uint32_t data_port=parse_int(getenv("NYNN_MM_DATASERV_PORT"),50000);
	uint32_t data_port_range_min=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MIN"),50000);
	uint32_t data_port_range_max=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MAX"),50008);
	for (int i=0;i<hosts.size();i++){
		if (hosts[i]==localhost)continue;
		uint32_t ip=host2ip(hosts[i]);
		uint32_t port=rand_range(data_port_range_min,data_port_range_max);
		string data_endpoint="tcp://"+ip2string(ip)+":"+to_string(port);
		datasocks[ip].reset(new zmq::socket_t(ctx,ZMQ_REQ));
		datasocks[ip]->connect(data_endpoint.c_str());
	}

	uint32_t localip=get_ip();
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	while(flag){
		zmq::poll(items.get(),socket_num,-1);
		for (int i=0;i<socket_num;i++){
			if (items[i].revents&ZMQ_POLLIN){
				prot::Replier rep(*sockets[i].get());
				rep.parse_ask();
				switch(rep.get_cmd()){
				case prot::CMD_WRITE:{
					handle_write_g(rep,*graph.get(),glock,datasocks);
					//auto p2h_write=handle_write_g;
					//syncw<void>(glock,p2h_write,rep,*graph.get(),datasocks);
					break;}
				case prot::CMD_READ:{
					handle_read(rep,*graph.get(),glock,localip,datasocks);
					//auto p2h_read=handle_read;
					//syncr<void>(glock,p2h_read,rep,*graph.get(),localip,datasocks);
					break;}
				case prot::CMD_NOTIFY:{
					handle_notify(rep,talkerid);
					//auto p2h_notity=handle_notify;
					break;}
				default:{
					log_w("ignore invalid request cmd");
					break;}
				};
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

void* talker(void* args)
{
	try
	{
	zmq::context_t& ctx=*(zmq::context_t*)args;
	zmq::socket_t namesock(ctx,ZMQ_REQ);
	uint32_t name_node=host2ip(getenv("NYNN_MM_NAMESERV_HOST"));
	uint32_t name_port_range_min=parse_int(getenv("NYNN_MM_NAMESERV_PORT_RANGE_MIN"),60000);
	uint32_t name_port_range_max=parse_int(getenv("NYNN_MM_NAMESERV_PORT_RANGE_MAX"),60008);
	uint32_t name_port=rand_range(name_port_range_min,name_port_range_max);
	string name_endpoint=string("tcp://")+ip2string(name_node)+":"+to_string(name_port);
	namesock.connect(name_endpoint.c_str());
	prot::Requester req(namesock);

	submit(req,*graph.get(),glock);
	//periodically pull ShardTable from namenode or be notified 
	//by namenode to pull fresh shard table from namenode.
	
	sigset_t mask;
	pthread_sigmask(SIG_BLOCK,NULL,&mask);
	sigaddset(&mask,SIGHUP);
	pthread_sigmask(SIG_SETMASK,&mask,NULL);

	int sfd=signalfd(-1,&mask,SFD_NONBLOCK);
	int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	struct itimerspec its={{5,0},{5,0}};
	timerfd_settime(tfd,0,&its,NULL);
	int efd=epoll_create(2);
	struct epoll_event ready_events[2],ev;
	ev.data.fd=sfd;
	ev.events=EPOLLIN;
	epoll_ctl(efd,EPOLL_CTL_ADD,sfd,&ev);
	ev.data.fd=tfd;
	ev.events=EPOLLIN;
	epoll_ctl(efd,EPOLL_CTL_ADD,tfd,&ev);
	char buff[sizeof(struct signalfd_siginfo)];
	uint32_t localip=get_ip();
	
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	while(flag){
		epoll_wait(efd,ready_events,2,-1);
		for(int i=0;i<2;i++){
			if(ready_events[i].events&EPOLLIN){
				read(ready_events[i].data.fd,buff,sizeof(buff));
				hello(req,*graph.get(),glock,localip);
				//auto p2hello=hello;
				//syncw<void>(glock,p2hello,req,*graph.get());
			}
		}
		flag=(intptr_t)pthread_getspecific(flag_key);
	}
	close(efd);
	close(sfd);
	close(tfd);
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
	graph.reset(new Graph(getenv("NYNN_MM_DATA_DIR"),get_ip()));
	//creating switcher,logger,worker_thds threads.
	zmq::context_t ctx;//ctx(io_threads)
	zmq_ctx_set(ctx,ZMQ_IO_THREADS,16);
	zmq_ctx_set(ctx,ZMQ_MAX_SOCKETS,4096);

	uint32_t port_range_min=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MIN"),50000);
	uint32_t port_range_max=parse_int(getenv("NYNN_MM_DATASERV_PORT_RANGE_MAX"),50008);
	uint32_t port_range_num=port_range_max-port_range_min;
	uint32_t hwm=parse_int(getenv("NYNN_MM_DATASERV_HWM"),1<<11);
	uint32_t buf=parse_int(getenv("NYNN_MM_DATASERV_BUF"),1<<16);
	uint64_t affinity=parse_int(getenv("NYNN_MM_DATASERV_AFFINITY"),(1<<2)-1);

	log_i("hwm=%d",hwm);
	log_i("buf=%d",buf);
	log_i("affinity=%d",affinity);
	
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
		gathers[i]->setsockopt(ZMQ_SNDBUF,&buf,sizeof(buf));
		gathers[i]->setsockopt(ZMQ_RCVBUF,&buf,sizeof(buf));
		gathers[i]->setsockopt(ZMQ_AFFINITY,&affinity,sizeof(affinity));
		
		scatters[i].reset(new zmq::socket_t(ctx,ZMQ_DEALER));
		scatters[i]->bind(scatter_endpoint.c_str());
		scatters[i]->setsockopt(ZMQ_SNDHWM,&hwm,sizeof(hwm));
		scatters[i]->setsockopt(ZMQ_RCVHWM,&hwm,sizeof(hwm));
		scatters[i]->setsockopt(ZMQ_SNDBUF,&buf,sizeof(buf));
		scatters[i]->setsockopt(ZMQ_RCVBUF,&buf,sizeof(buf));
		scatters[i]->setsockopt(ZMQ_AFFINITY,&affinity,sizeof(affinity));
		X x={gathers[i].get(),scatters[i].get()};
		switcher_thds[i].reset(new thread_t(switcher,&x));
		switcher_thds[i]->start();
	}
	log_i("create %d switcher for serv port from %d to %d",port_range_num,port_range_min,port_range_max);
#if 0
	zmq::socket_t collector(ctx,ZMQ_ROUTER);
	zmq::socket_t dispatcher(ctx,ZMQ_DEALER);
	
	uint32_t port=parse_int(getenv("NYNN_MM_DATASERV_PORT"),40001);
	string collector_endpoint=string("tcp://")+ ip2string(get_ip())+ ":"+to_string(port); 
	log_i("collector endpoint: %s",collector_endpoint.c_str());

	//create switcher
	collector.bind(collector_endpoint.c_str());
	dispatcher.bind("inproc://dispatcher.inproc");
	X x={&collector,&dispatcher};
	thread_t switcher_thd(switcher,&x);
	switcher_thd.start();
#endif 
	//create worker_thds
	uint32_t workerNum=parse_int(getenv("NYNN_MM_DATASERV_WORKER_NUM"),3);
	unique_ptr<thread_t> *worker_thds=new unique_ptr<thread_t>[workerNum];
	for (int i=0;i<workerNum;i++)worker_thds[i].reset(new thread_t(worker,&ctx));
	for (int i=0;i<workerNum;i++)worker_thds[i]->start();

	//create talker
	thread_t talker_thd(talker,&ctx);
	talker_thd.start();
	talkerid=talker_thd.thread_id();

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
	
	//shutdown switchers gracefully.
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

#if 0	
	//shutdown switcher gracefully.
	collector.close();
	dispatcher.close();
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.kill(SIGTERM);
	nanosleep_for(5000);
	if (switcher_thd.is_alive())switcher_thd.stop();
	switcher_thd.join();
	log_i("switcher is shutdown");

	if (talker_thd.is_alive())talker_thd.kill(SIGTERM);
	nanosleep_for(5000);
	if (talker_thd.is_alive())talker_thd.stop();
	talker_thd.join();
#endif
}
