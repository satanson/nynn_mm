#include<nynn_file.hpp>
#include<nynn_fs.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv){
	string naddr=argv[1];
	string daddr=argv[2];
	uint32_t vtxno=parse_int(argv[3],~0u);
	string p=argv[4];

	Next next = p=="pop"?
				&Block::BlockHeader::getPrev
				:
				&Block::BlockHeader::getNext;

	nynn_fs fs(naddr,daddr);
	nynn_file f(fs,vtxno);


	//uint32_t blkno = (p=="pop"?(nynn_file::tailblkno):(nynn_file::headblkno));
	uint32_t blkno=nynn_file::invalidblkno;
	if (p=="pop")blkno=nynn_file::tailblkno;
	else blkno=nynn_file::headblkno;
	while(blkno!=nynn_file::invalidblkno){
		shared_ptr<Block> blk=f.read(blkno);
		blkno=(blk->getHeader()->*next)();
		CharContent *cctt=*blk.get();
		string line(cctt->begin(),cctt->end());
		cout<<line<<endl;
	}
}
