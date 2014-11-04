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
uint32_t port;
uint32_t thdsz;

void* reader(void*arg){
	int N=(intptr_t)arg;

	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	
	string endpoint=string("tcp://")+ip+":"+to_string(port);
	sock.connect(endpoint.c_str());

	prot::Requester req(sock);

	uint32_t w=SubgraphSet::VERTEX_INTERVAL_WIDTH;
	uint32_t vtxno_beg=w*N;
	uint32_t vtxno_end=w*(N+1);
	uint32_t cnt=0;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	Block blk;
	EdgeContent *ectt=blk;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for (uint32_t vtxno=vtxno_beg;vtxno<vtxno_end && cnt<w;vtxno++) {
		uint32_t blkno=HEAD_BLOCKNO;
		while(blkno!=INVALID_BLOCKNO){
			if(!read(req,vtxno,blkno,&blk))break;
			blkno=blk.getHeader()->getNext();
			concurrency+=ectt->end()-ectt->begin();
			nbytes+=sizeof(Block);
		}
		++cnt;
		if (cnt%100==0) cout<<cnt<<endl;
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
	port=parse_int(argv[2],61000);
	thdsz=parse_int(argv[3],16);

	ZMQSockArray socks(new  ZMQSock[thdsz]);
	ThreadArray threads(new Thread[thdsz]);
	string endpoint="tcp://"+ip+":"+to_string(port);

	for(int i=0;i<thdsz;i++){
		threads[i].reset(new thread_t(reader,(void*)i));
	}
	sleep(1);
	for (int i=0;i<thdsz;i++)threads[i]->start();
	for (int i=0;i<thdsz;i++)threads[i]->join();
	return 0;
}
