#include<nynn_mm_config.hpp>
#include<sys/time.h>

typedef struct{
	SubgraphSet *sgs;
	string path;
}WriteOrder;

typedef unique_ptr<thread_t> Thread;
typedef unique_ptr<Thread[]> ThreadArray;

typedef uint32_t (SubgraphSet::*Action)(uint32_t,Block*);
Action act;
vector<string> lines;
vector<WriteOrder> orders;
uint32_t loop;
uint32_t thdsz;

void* writer(void* arg){
	WriteOrder &order=*(WriteOrder*)arg;
	SubgraphSet& sgs=*order.sgs;
	string& path=order.path;

	MmapFile mf(path);
	char *begin=(char*)mf.getBaseAddress();
	char *end=begin+mf.getLength();

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	Block blk;
	EdgeContent *ectt=blk;
	uint32_t max_nedges=EdgeContent::CONTENT_CAPACITY;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	char *p=begin;
	while(p<end){
		uint32_t vtxno=*(uint32_t*)p;
		p+=sizeof(uint32_t);
		uint32_t nedges=*(uint32_t*)p;
		p+=sizeof(uint32_t);
		Edge *eb=(Edge*)p;
		Edge *ee=eb+nedges;
		Edge *e=eb;
		p=(char*)ee;
		concurrency+=nedges;
		//if (concurrency%10000==0)cout<<concurrency/10000<<endl;
		nbytes+=sizeof(vtxno)+sizeof(nedges)+sizeof(Edge)*nedges;
		while(e+max_nedges<ee){
			ectt->resize(max_nedges);
			std::copy(e,e+max_nedges,ectt->begin());
			e+=max_nedges;
			sgs.push(vtxno,&blk);
		}
		if (e!=ee){
			ectt->resize(ee-e);
			std::copy(e,ee,ectt->begin());
			sgs.push(vtxno,&blk);
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
	string basedir=argv[1];
	thdsz=parse_int(argv[2],16);
	string base=argv[3];

	SubgraphSet sgs(basedir);

	ThreadArray threads(new Thread[thdsz]);
	orders.resize(thdsz);

	for(int i=0;i<thdsz;i++){
		orders[i].sgs=&sgs;
		orders[i].path=base+"."+to_string(i);
		threads[i].reset(new thread_t(writer,&orders[i]));
	}
	sleep(1);
	for (int i=0;i<thdsz;i++)threads[i]->start();
	for (int i=0;i<thdsz;i++)threads[i]->join();
	return 0;
}

