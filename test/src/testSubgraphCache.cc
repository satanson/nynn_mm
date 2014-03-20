#include<test.h>
using namespace nynn::mm;

int main(int argc,char**argv)
{
	unique_ptr<GraphCache> cache(new GraphCache());
	uint32_t access=0;
	uint32_t hit=0;
	Block blk;
	uint32_t N=atoi(argv[1]);
	uint32_t V=atoi(argv[2]);
	uint32_t B=atoi(argv[3]);
	uint32_t K=atoi(argv[4]);
	for (uint32_t i=0;i<N;i++){
		uint32_t vtxno=rand_int()%V;
		uint32_t blkno=rand_int()%B;
		if(!cache->read(vtxno,blkno,&blk)){
			for(uint32_t i=0;i<K;i++){
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
