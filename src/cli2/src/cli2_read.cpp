#include<nynn_fs.hpp>
#include<nynn_file.hpp>
typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string naddr=argv[1];
	string daddr=argv[2];
	uint32_t vtxno_begin=parse_int(argv[3],~0u);
	uint32_t vtxno_end=parse_int(argv[4],~0u);
	string actid=argv[5];
	uint32_t loop=parse_int(argv[6],~0u);

	assert(vtxno_begin!=~0u);
	assert(vtxno_end!=~0u);
	assert(actid=="pop"||actid=="shift");

	Next next = actid=="pop"?
				&Block::BlockHeader::getPrev
				:
				&Block::BlockHeader::getNext;

	nynn_fs fs(naddr,daddr);

	uint32_t firstblkno=nynn_file::invalidblkno;
	if (actid=="pop")firstblkno=nynn_file::tailblkno;
	else firstblkno=nynn_file::headblkno;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;

	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	for (int i=0;i<loop;i++)
	for (uint32_t vtxno=vtxno_begin;vtxno<vtxno_end;vtxno++) {
		nynn_file f(fs,vtxno);
		uint32_t blkno=firstblkno;
		while(blkno!=nynn_file::invalidblkno){
			shared_ptr<Block> blk=f.read(blkno);
			blkno=(blk->getHeader()->*next)();
			concurrency++;
			nbytes+=sizeof(Block);
		}
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
