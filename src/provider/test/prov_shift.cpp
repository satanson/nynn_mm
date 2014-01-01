#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);
	uint32_t vtxno=strtoul(argv[3],NULL,0);
	uint32_t erase=strtoul(argv[4],NULL,0); 

	ProviderRPC prov(host,port);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);
	for (int i=0;i<keys.size();i++)prov.attachSubgraph(keys[i]);

	RawBlock rawblk;
	Block* blk=rawblk;//for presentation
	vector<int8_t> &xblk=rawblk;//for exchanging
	CharContent *content=*blk;

	prov.lock(vtxno,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);

	uint32_t blkno=prov.getHeadBlkno(vtxno);
	while(blkno!=INVALID_BLOCKNO){
		prov.read(vtxno,blkno,xblk);
		string output(content->begin(),content->end());
		cout<<"vtx="<<vtxno<<", blkno="<<blkno<<": "<<output<<endl;
		blkno=blk->getHeader()->getNext();
	}
	if(erase)while(prov.shift(vtxno)!=INVALID_BLOCKNO);

	prov.unlock(vtxno);
}
