#include<nynn_mm_config.h>
#include<AgentRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string agentHost=argv[1];
	uint32_t agentPort=strtoul(argv[2],NULL,0);
	string file=argv[3];
	uint32_t maxVtxno=strtoul(argv[4],NULL,0);
	std::shared_ptr<AgentRPC> agent(new AgentRPC(agentHost,agentPort));
	ofstream fout(file);
	RawBlock rblk;
	vector<int8_t>& xblk=rblk;
	Block* blk=rblk;
	CharContent *content=*blk;
	uint32_t vtxno=0;
	while(vtxno<maxVtxno){
		uint32_t blkno=agent->getHeadBlkno(vtxno);
		agent->read(vtxno,blkno,xblk);
		string line(content->begin(),content->end());
		fout<<line<<endl;
	}
	fout.close();
	
}
