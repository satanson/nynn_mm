#include<nynn_mm_config.hpp>
#include<sys/time.h>

typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vtxnoBeg=strtoul(argv[3],NULL,0);
	uint32_t vtxnoEnd=strtoul(argv[4],NULL,0);
	uint32_t firstBlkno=actid==string("shift")?HEAD_BLOCKNO:TAIL_BLOCKNO;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	Next next=nexts[actid];

	uint32_t sgkeyBeg=vtxnoBeg-vtxnoBeg%SubgraphSet::VERTEX_INTERVAL_WIDTH;

	SubgraphSet sgs(basedir);

	struct timeval beg_tv,end_tv;
	double t;
	gettimeofday(&beg_tv,NULL);
	
	for (uint32_t sgkey=sgkeyBeg;sgkey<vtxnoEnd;sgkey+=SubgraphSet::VERTEX_INTERVAL_WIDTH){
		sgs.attachSubgraph(sgkey);
	}

	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"attach subgraph:["<<vtxnoBeg<<","<<vtxnoEnd<<")"<<endl;
	cout<<"time usage:"<<t<<endl;

	Block blk;
	CharContent *content=blk;

	gettimeofday(&beg_tv,NULL);
	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++) {
	
		uint32_t blkno=firstBlkno;
		while (blkno!=INVALID_BLOCKNO){
			sgs.read(vtxno,blkno,&blk);
			blkno=(blk.getHeader()->*next)();
		};
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"write vtxno("<<vtxnoEnd-vtxnoBeg<<"): ["<<vtxnoEnd<<","<<vtxnoBeg<<")"<<endl;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"vtxno per second="<<(vtxnoEnd-vtxnoBeg)/t<<endl;
}
