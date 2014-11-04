#include<nynn_mm_config.hpp>
#include<sys/time.h>

typedef uint32_t (SubgraphSet::*Action)(uint32_t,Block*);

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vbegin=strtoul(argv[3],NULL,0);
	uint32_t vend=strtoul(argv[4],NULL,0);
	string file=argv[5];

	map<string,Action> actions;
	actions["push"]=&SubgraphSet::push;
	actions["unshift"]=&SubgraphSet::unshift;
	Action act=actions[actid];

	if (act==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	SubgraphSet sgs(basedir);

	ifstream fin(file);
	vector<string> lines;
	string line;
	while(getline(fin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)
			line.resize(CharContent::CONTENT_CAPACITY);

		lines.push_back(line);
	}

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	Block blk;
	CharContent *cctt=blk;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for (uint32_t vtxno=vbegin;vtxno<vend;vtxno++) {
		if (unlikely(!sgs.exists(vtxno))){
			uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
			sgs.createSubgraph(sgkey);
		}
		for (uint32_t ln=0;ln<lines.size();ln++){
			string& line=lines[ln];
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
}
