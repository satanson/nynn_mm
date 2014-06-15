#include<nynn_mm_config.hpp>
int main(int argc,char**argv){
	string basedir=argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	uint32_t eb=parse_int(argv[3],0);
	uint32_t ee=parse_int(argv[4],1024);
	SubgraphSet sgs(basedir);
	
	Block blk,*retBlk;
	EdgeContent *ectt=blk;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	
	for(uint32_t i=eb;i<ee;i++){
		retBlk=sgs.read(vtxno,TAIL_BLOCKNO,&blk);
		if (unlikely(retBlk==NULL ||ectt->size()==EdgeContent::CONTENT_CAPACITY)){
			ectt->resize(1);
			Edge *e=ectt->end()-1;
			e->m_sink=i;
			e->m_weight.m_fval=3.1415926;
			e->m_timestamp=0;
			sgs.push(vtxno,&blk);
		}else{
			ectt->resize(ectt->size()+1);
			Edge *e=ectt->end()-1;
			e->m_sink=i;
			e->m_weight.m_fval=3.1415926;
			e->m_timestamp=0;
			sgs.pop(vtxno);
			sgs.push(vtxno,&blk);
		}

	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	uint32_t n=ee-eb;
	cout<<"timeusage:"<<t<<endl;
	cout<<"nbytes:"<<n*sizeof(Edge)<<endl;
	cout<<"throughput:"<<n*sizeof(Edge)/(t+0.0)<<endl;
}
