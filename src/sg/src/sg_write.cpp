#include<nynn_mm_config.h>
#include<sys/time.h>

typedef uint32_t (SubgraphSet::*Action)(uint32_t,Block*);

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vtxnoBeg=strtoul(argv[3],NULL,0);
	uint32_t vtxnoEnd=strtoul(argv[4],NULL,0);
	string file=argv[5];

	map<string,Action> actions;
	actions["push"]=&SubgraphSet::push;
	actions["unshift"]=&SubgraphSet::unshift;
	Action act=actions[actid];

	if (act==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	uint32_t sgkeyBeg=vtxnoBeg-vtxnoBeg%SubgraphSet::VERTEX_INTERVAL_WIDTH;

	SubgraphSet sgs(basedir);

	struct timeval beg_tv,end_tv;
	double t;
	gettimeofday(&beg_tv,NULL);
	
	for (uint32_t sgkey=sgkeyBeg;sgkey<vtxnoEnd;sgkey+=SubgraphSet::VERTEX_INTERVAL_WIDTH){
		sgs.createSubgraph(sgkey);
		sgs.attachSubgraph(sgkey);
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"create subgraph:["<<vtxnoBeg<<","<<vtxnoEnd<<")"<<endl;
	cout<<"time usage:"<<t<<endl;

	ifstream fin(file);
	vector<string> lines;
	string line;
	while(getline(fin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)
			line.resize(CharContent::CONTENT_CAPACITY);

		lines.push_back(line);
	}

	Block blk;
	CharContent *content=blk;

	gettimeofday(&beg_tv,NULL);
	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++) {
		for (uint32_t ln=0;ln<lines.size();ln++){
			string& line=lines[ln];
			content->resize(line.size());
			std::copy(line.begin(),line.end(),content->begin());
			(sgs.*act)(vtxno,&blk);
		}
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"write vtxno("<<vtxnoEnd-vtxnoBeg<<"): ["<<vtxnoEnd<<","<<vtxnoBeg<<")"<<endl;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"vtxno per second="<<(vtxnoEnd-vtxnoBeg)/t<<endl;
}
