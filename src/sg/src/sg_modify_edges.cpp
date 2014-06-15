#include<nynn_mm_config.hpp>
#include<nynn_common.hpp>
using namespace std;
using namespace nynn;
int main(int argc,char**argv){
	string basedir=argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	uint32_t eb=parse_int(argv[3],0);
	uint32_t ee=parse_int(argv[4],1024);
	uint32_t n=parse_int(argv[5],4096);

	SubgraphSet sgs(basedir);
	
	Block blk,*retBlk;
	EdgeContent *ectt=blk;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for(uint32_t i=0;i<n;i++){
		uint32_t blkno=HEAD_BLOCKNO;
		uint32_t ei=rand_range(eb,ee);
		while((retBlk=sgs.read(vtxno,blkno,&blk))!=NULL){
			uint32_t i=0;
			for(i=0;i<ectt->size();i++){
				Edge *e=ectt->pos(i);
				if(e->m_sink!=ei)continue;
				else{
#ifdef DEBUG
					cout<<"["<<e->m_sink<<","<<e->m_weight.m_fval<<","<<e->m_timestamp<<"]"
						<<"->"
						<<"["<<e->m_sink<<","<<e->m_weight.m_fval*2<<","<<e->m_timestamp+1<<"]"<<endl;
#endif
					e->m_weight.m_fval*=2;
					e->m_timestamp+=1;
					sgs.write(vtxno,blkno,&blk);
					break;
				}
			}	
			if(i==ectt->size())blkno=blk.getHeader()->getNext();
			else break;
		}

	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	cout<<"timeusage:"<<t<<endl;
	cout<<"nbytes:"<<n*sizeof(Edge)<<endl;
	cout<<"throughput:"<<n*sizeof(Edge)/(t+0.0)<<endl;
}
