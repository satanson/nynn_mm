#include<nynn_mm_config.h>
#include<ProducerRPC.h>
#include<sys/time.h>
using namespace nynn::mm::rpc;
typedef int32_t (ProducerRPC::*Action)(int32_t,const vector<int8_t>&);
int main(int argc, char**argv)
{
	string actid=argv[1];
	string prodHost=argv[2];
	uint32_t prodPort=strtoul(argv[3],NULL,0);
	uint32_t vtxnoBeg=strtoul(argv[4],NULL,0);
	uint32_t vtxnoEnd=strtoul(argv[5],NULL,0);
	string file=argv[6];

	map<string,Action> actions;
	actions["push"]=&ProducerRPC::push;
	actions["unshift"]=&ProducerRPC::unshift;
	Action act=actions[actid];

	if (act==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	std::shared_ptr<ProducerRPC> prod(new ProducerRPC(prodHost,prodPort));

	ifstream fin(file);
	vector<string> lines;
	string line;
	while(getline(fin,line)){
		lines.push_back(line);
	}

	RawBlock rblk;
	vector<int8_t>& xblk=rblk;
	Block* blk=rblk;
	CharContent *content=*blk;

	struct timeval beg_tv,end_tv;
	gettimeofday(&beg_tv,NULL);
	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++){
		for (uint32_t ln=0;ln<lines.size();ln++){
			string &line=lines[ln];
			if(line.size()>CharContent::CONTENT_CAPACITY)
				line.resize(CharContent::CONTENT_CAPACITY);
			content->resize(line.size());
			std::copy(line.begin(),line.end(),content->begin());
			(prod.get()->*act)(vtxno,xblk);
		}
	}
	gettimeofday(&end_tv,NULL);
	double t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-
		   (beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"write vtxno:<<"<<vtxnoEnd-vtxnoBeg<<endl;
	cout<<"vtxno per second="<<(vtxnoEnd-vtxnoBeg)/t<<endl;
}

