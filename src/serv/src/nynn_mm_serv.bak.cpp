#include<zmq.hpp>
#include<sys/signalfd.h>
#include<nynn_mm_common.h>
using namespace nynn::mm::common;
using namespace std;
typedef struct{
	zmq::context_t *pctx;
	zmq::socket_t *pfrontend;
	zmq::socket_t *pbackend;
}dualSocket;

void* switcher(void* args)
{
	dualSocket* pdual=(dualSocket*)args;
	zmq::socket_t sniffer(*pdual->pctx,ZMQ_PAIR);
	sniffer.bind("inproc://sniffer");
	zmq::proxy(*pdual->pfrontend,*pdual->pbackend,sniffer);
	sniffer.close();
	return NULL;
}

void* logger(void*args)
{
	zmq::context_t *pctx=(zmq::context_t*)args;
	zmq::socket_t recruit(*pctx,ZMQ_SUB);
	recruit.connect("inproc://veteran");
	recruit.setsockopt(ZMQ_SUBSCRIBE,"CMD",3);
	zmq::message_t msg;
	recruit.recv(msg,0);
	char cmd[64];
	strncpy(cmd,(const char*)msg.data(),msg.size());

	zmq::socket_t recorder(*pctx,ZMQ_PAIR);
	recorder.connect("inproc://logger");

	return NULL;
}

void* worker(void*args)
{
	zmq::context_t *pctx=(zmq::context_t*)args;
	zmq::socket_t recruit(*pctx,ZMQ_SUB);
	recruit.connect("inproc://veteran");
	recruit.setsockopt(ZMQ_SUBSCRIBE,"CMD",3);
	
	return NULL;
}

uint32_t parseInt(const char* s,uint32_t value){
	if (s==NULL){
		log_w("s shouldn't be NULL");
		return value;
	}
	uint32_t value2=strtoul(s,NULL,0);
	if (value2==0||value2==ULONG_MAX){
		log_w("fail to convert %s to uint32_t",s);
		return value;
	}
	return value2;
}

int main(){
	uint32_t port=parseInt(getenv("NYNN_MM_SERV_PORT"),40001);
	uint32_t workerNum=parseInt(getenv("NYNN_MM_SERV_PORT"),3);
	zmq::context_t context;
	pthread_t switcherId,loggerId,*workerIds;
	workerIds=new pthread_t[workerNum];

	int errnum=0;
	pthread_create(&switcherId,NULL,switcher,&context);
	pthread_create(&loggerId,NULL,logger,&context);
	
	for (int i=0;i<workerNum;i++){
		pthread_create(&workerIds[i],NULL,worker,&context);
	}
	
	zmq::socket_t monitor(context,ZMQ_PUB);
	monitor.bind("inproc://monitor");

	const char* CMD_START="CMD_START";
	const char* CMD_STOP="CMD_STOP";
	batman.send(CMD_START,strlen(CMD_START)+1);

	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	sigemptyset(&mask);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGTERM);

	if (sigprocmask(SIG_BLOCK,&mask,NULL)==-1)
		throwNynnException(errno,"fail to invoke sigprocmask");
	sfd = signalfd(-1,&mask,0);
	if (sfd == -1)
		throwNynnException(errno,"fail to create signalfd");

	ssize_t s = read(sfd,&fdsi,sizeof(fdsi));
	if (s != sizeof(fdsi))
		throwNynnException(errno,"fail to read signal from signalfd");
	string sig;
	switch(fdsi.ssi_signo){
		case SIGINT:
			sig="SIGINT";break;
		case SIGQUIT:
			sig="SIGQUIT";break;
		case SIGTERM:
			sig="SIGTERM";break;
		default:
			sig="unknown signal";
	}
	log_i("nynn_mm_serv terminated by %s",sig.c_str());

	batman.send(CMD_STOP,strlen(CMD_STOP)+1);
	pthread_join(switcherId,NULL);
	pthread_join(loggerId,NULL);
	for (int i=0;i<workerNum;i++){
		pthread_join(workerIds[i],NULL);
	}
}
