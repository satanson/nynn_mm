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

	assert(vtxno_begin!=~0u);
	assert(vtxno_end!=~0u);

	nynn_fs fs(naddr,daddr);

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;

	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	int cnt=0;
	for (uint32_t vtxno=vtxno_begin;vtxno<vtxno_end;vtxno++) {
		nynn_file f(fs,vtxno);
		uint32_t blkno=f.getheadblkno();
		while(blkno!=nynn_file::invalidblkno){
			shared_ptr<Block> blk=f.read(blkno);
			assert(blk.get()!=NULL);
			blkno=blk->getHeader()->getNext();
			EdgeContent *ectt=*blk.get();
			concurrency+=ectt->end()-ectt->begin();
			++cnt;
			if (cnt%100000==0)cout<<cnt<<endl;
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
