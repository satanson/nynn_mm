#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>

typedef unique_ptr<zmq::socket_t> ZMQSock;
typedef unique_ptr<ZMQSock[]> ZMQSockArray;
typedef unique_ptr<thread_t> Thread;
typedef unique_ptr<Thread[]> ThreadArray;

string ip;
uint32_t port_min;
uint32_t port_max;
uint32_t vtxno_beg;
uint32_t vtxno_end;
uint32_t loop;
uint32_t thdsz;

void* reader(void*arg){
	zmq::socket_t& sock=*(zmq::socket_t*)arg;
	prot::Requester req(sock);
	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	Block blk;
	CharContent *cctt=blk;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	for (int i=0;i<loop;i++)
	for (uint32_t vtxno=vtxno_beg;vtxno<vtxno_end;vtxno++) {
		uint32_t blkno=HEAD_BLOCKNO;
		while(blkno!=INVALID_BLOCKNO){
			if(!read(req,vtxno,blkno,&blk))break;
			blkno=blk.getHeader()->getNext();
			concurrency++;
			nbytes+=sizeof(Block);
		}
	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	ofstream out(format("output.%d",pthread_self()));
	out.setf(ios::fixed);
	out<<"tbegin:"<<tbegin<<endl;
	out<<"tend:"<<tend<<endl;
	out<<"t:"<<t<<endl;
	out<<"nbytes:"<<nbytes<<endl;
	out<<"concurrency:"<<concurrency<<endl;
	out<<"throughput:"<<nbytes/1024.0/1024.0<<endl;
	out<<"ave_concurrency:"<<concurrency/t<<endl;
	out<<"ave_throughput:"<<nbytes/t/1024.0/1024.0<<endl;
	pthread_exit(NULL);
}

int main(int argc,char**argv)
{
	ip=argv[1];
	port_min=parse_int(argv[2],60000);
	port_max=parse_int(argv[3],60001);
	vtxno_beg=parse_int(argv[4],~0u);
	vtxno_end=parse_int(argv[5],~0u);
	thdsz=parse_int(argv[6],16);
	loop=parse_int(argv[7],1024);

	assert(vtxno_beg!=~0u);
	assert(vtxno_end!=~0u);

	zmq::context_t ctx;
	ZMQSockArray socks(new  ZMQSock[thdsz]);
	ThreadArray threads(new Thread[thdsz]);
	for(int i=0;i<thdsz;i++){
		socks[i].reset(new zmq::socket_t(ctx,ZMQ_REQ));
		string endpoint="tcp://"+ip+":"+to_string(port_min+i%(port_max-port_min));
		socks[i]->connect(endpoint.c_str());
		threads[i].reset(new thread_t(reader,socks[i].get()));
	}
	sleep(1);
	for (int i=0;i<thdsz;i++)threads[i]->start();
	for (int i=0;i<thdsz;i++)threads[i]->join();
	return 0;
}
