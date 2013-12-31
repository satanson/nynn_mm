#include<test.h>
using namespace nynn::mm;

int main(int argc,char**argv)
{
	printf("pre SubgraphCache\n");
	unique_ptr<GraphCache> cache(new GraphCache());
	printf("post SubgraphCache\n");
	uint32_t access=0;
	uint32_t hit=0;
	Block blk;
	uint32_t N=atoi(argv[1]);
	uint32_t M=atoi(argv[2]);
	uint32_t K=atoi(argv[3]);
	for (uint32_t i=0;i<N;i++){
		uint32_t vtxno=rand_int()%M;
		uint32_t blkno=rand_int()%K;
		if(!cache->read(vtxno,blkno,&blk)){
			for(uint32_t i=0;i<8;i++){
			 	cache->write(vtxno,blkno+i,&blk);
			}
			cache->read(vtxno,blkno,&blk);
		}else{
			hit++;
		}
		access++;
	}
	cout<<"hit rate="<<(hit+0.0)/access<<endl;
}
