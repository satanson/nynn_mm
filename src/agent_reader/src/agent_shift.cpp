#include<nynn_mm_config.h>
#include<AgentRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string agentHost=argv[1];
	uint32_t agentPort=strtoul(argv[2],NULL,0);
	uint32_t vtxno=strtoul(argv[3],NULL,0);

	std::shared_ptr<AgentRPC> agent(new AgentRPC(agentHost,agentPort));

	RawBlock rblk;
	vector<int8_t>& xblk=rblk;
	Block* blk=rblk;
	CharContent *content=*blk;

	agent->lock(vtxno);

	uint32_t blkno=agent->getHeadBlkno(vtxno);
	while(blkno!=INVALID_BLOCKNO){
		agent->read(vtxno,blkno,xblk);
		string output(content->begin(),content->end());
		cout<<output<<endl;
		blkno=blk->getHeader()->getNext();
	}

	agent->unlock(vtxno);
}
