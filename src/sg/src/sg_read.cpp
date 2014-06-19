#include<linuxcpp.hpp>
#include<nynn_mm_config.hpp>

typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vtxnoBeg=parse_int(argv[3],0);
	uint32_t vtxnoEnd=parse_int(argv[4],1024);
	uint32_t loop=parse_int(argv[5],16);

	uint32_t firstBlkno=actid==string("shift")?HEAD_BLOCKNO:TAIL_BLOCKNO;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	Next next=nexts[actid];

	uint32_t sgkeyBeg=vtxnoBeg-vtxnoBeg%SubgraphSet::VERTEX_INTERVAL_WIDTH;

	SubgraphSet sgs(basedir);

	
	for (uint32_t sgkey=sgkeyBeg;sgkey<vtxnoEnd;sgkey+=SubgraphSet::VERTEX_INTERVAL_WIDTH){
		sgs.attachSubgraph(sgkey);
	}

	Block blk,*retblk;
	CharContent *content=blk;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for (int i=0;i<loop;i++)
	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++) {
	
		uint32_t blkno=firstBlkno;
		while (blkno!=INVALID_BLOCKNO){
			retblk=sgs.read(vtxno,blkno,&blk);
			if (unlikely(retblk==NULL)) return 0;
			blkno=(retblk->getHeader()->*next)();
			concurrency++;
			nbytes+=sizeof(Block);
		};
	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	cout.setf(ios::fixed);
	cout<<"tbegin:"<<tbegin<<endl;
	cout<<"tend:"<<tend<<endl;
	cout<<"t:"<<t<<endl;
	cout<<"nbytes:"<<nbytes<<endl;
	cout<<"concurrency:"<<concurrency<<endl;
	cout<<"throughput:"<<nbytes/1024.0/1024.0<<endl;
	cout<<"ave_concurrency:"<<concurrency/t<<endl;
	cout<<"ave_throughput:"<<nbytes/t/1024.0/1024.0<<endl;
}
