#include<nynn_mm_config.hpp>
int main(int argc,char**argv){
	string basedir=argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	uint32_t eb=parse_int(argv[3],0);
	uint32_t ee=parse_int(argv[4],1024);
	SubgraphSet sgs(basedir);
	int n=0;
	Block blk,*retBlk;
	EdgeContent *ectt=blk;
	uint32_t blkno=HEAD_BLOCKNO;
	while((retBlk=sgs.read(vtxno,blkno,&blk))!=NULL){
		cout<<"size="<<ectt->size()<<endl;
		for(uint32_t i=0;i<ectt->size();i++){
			n++;
			if(n<eb||n>=ee)continue;
			Edge *e=ectt->pos(i);
			cout<<n<<":["<<e->m_sink<<","<<e->m_weight.m_fval<<","<<e->m_timestamp<<"]"<<endl;
		}
		blkno=blk.getHeader()->getNext();
	}
}
