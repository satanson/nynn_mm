#include<nynn_mm_config.hpp>
#include<sys/time.h>

typedef struct{
	SubgraphSet *sgs;
	uint32_t vbegin;
	uint32_t vend;
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
	uint32_t vbegin=order.vbegin;
	uint32_t vend=order.vend;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	Block blk;
	CharContent *cctt=blk;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	for (int i=0;i<loop;i++)
	for (uint32_t vtxno=vbegin;vtxno<vend;vtxno++) {
		if (unlikely(!sgs.vtx_exists(vtxno))){
			uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
			sgs.createSubgraph(sgkey);
		}
		for (uint32_t i=0;i<lines.size();i++){
			string& line=lines[i];
			cctt->resize(line.size());
			std::copy(line.begin(),line.end(),cctt->begin());
			(sgs.*act)(vtxno,&blk);
			concurrency++;
			nbytes+=CharContent::BLOCK_CONTENT_SIZE;
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
	uint32_t vbegin=parse_int(argv[3],0);
	uint32_t vend=parse_int(argv[4],1024);
	loop=parse_int(argv[5],1);
	uint32_t overlap=parse_int(argv[6],0);
	string actid=argv[7];
	string file=argv[8];

	map<string,Action> actions;
	actions["push"]=&SubgraphSet::push;
	actions["unshift"]=&SubgraphSet::unshift;
	act=actions[actid];

	if (act==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}
	SubgraphSet sgs(basedir);
	ifstream fin(file);
	string line;
	while(getline(fin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)
			line.resize(CharContent::CONTENT_CAPACITY);
		lines.push_back(line);
	}
	ThreadArray threads(new Thread[thdsz]);
	orders.resize(thdsz);

	for(int i=0;i<thdsz;i++){
		orders[i].sgs=&sgs;
		if (overlap){
			orders[i].vbegin=vbegin;
			orders[i].vend=vend;
		}else{
			orders[i].vbegin=vbegin+(vend-vbegin)/thdsz*i;
			orders[i].vend=vbegin+(vend-vbegin)/thdsz*(i+1);
		}
		log_i("thread %d vbegin=%d vend=%d",i,orders[i].vbegin,orders[i].vend);
		threads[i].reset(new thread_t(writer,&orders[i]));
	}
	sleep(1);
	for (int i=0;i<thdsz;i++)threads[i]->start();
	for (int i=0;i<thdsz;i++)threads[i]->join();
	return 0;
}

