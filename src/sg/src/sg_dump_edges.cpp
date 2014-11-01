#include<nynn_mm_config.hpp>
int main(int argc,char**argv){
	string basedir=argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	uint32_t eb=parse_int(argv[3],0);
	uint32_t ee=parse_int(argv[4],1024);
	cout<<basedir<<endl;
	SubgraphSet sgs(basedir);
	Subgraph *sg=sgs.getSubgraph(vtxno).get();
	Vertex *vtx=sg->getVertex(vtxno);

	int n=0;
	Block *blk;
	EdgeContent *ectt;
	uint32_t blkno=vtx->getHeadBlkno();

	while(blkno!=INVALID_BLOCKNO){
		blk=sg->getBlock(blkno);
		ectt=*blk;
		cout<<format("[blkno=%u, size=%u]",blkno,ectt->size())<<endl;
		for(uint32_t i=0;i<ectt->size();i++){
			n++;
			if(n<eb||n>=ee)continue;
			Edge *e=ectt->pos(i);
			cout<<n<<endl;
		}
		blkno=blk->getHeader()->getNext();
	}
}
