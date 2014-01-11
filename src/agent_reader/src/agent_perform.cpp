#include<nynn_mm_config.h>
#include<AgentRPC.h>
#include<sys/time.h>
using namespace nynn::mm::rpc;
typedef int32_t (AgentRPC::*First)(int32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string agentHost=argv[2];
	uint32_t agentPort=strtoul(argv[3],NULL,0);
	uint32_t vtxnoBeg=strtoul(argv[4],NULL,0);
	uint32_t vtxnoEnd=strtoul(argv[5],NULL,0);

	map<string,First> firsts;
	firsts["pop"]=&AgentRPC::getTailBlkno;
	firsts["shift"]=&AgentRPC::getHeadBlkno;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	First first=firsts[actid];
	Next next=nexts[actid];
	
	if (first==NULL || next==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	std::shared_ptr<AgentRPC> agent(new AgentRPC(agentHost,agentPort));

	RawBlock rblk;
	vector<int8_t>& xblk=rblk;
	Block* blk=rblk;
	CharContent *content=*blk;

	struct timeval beg_tv,end_tv;
	gettimeofday(&beg_tv,NULL);

	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++){
		agent->lock(vtxno);

		uint32_t blkno=(agent.get()->*first)(vtxno);
		while(blkno!=INVALID_BLOCKNO){
			agent->read(vtxno,blkno,xblk);
			blkno=(blk->getHeader()->*next)();
		}

		agent->unlock(vtxno);
	}
	gettimeofday(&end_tv,NULL);
	double t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-
		   (beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"read vtxno:<<"<<vtxnoEnd-vtxnoBeg<<endl;
	cout<<"vtxno per second="<<(vtxnoEnd-vtxnoBeg)/t<<endl;
}
