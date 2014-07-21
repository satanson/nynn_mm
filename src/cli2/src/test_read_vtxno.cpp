#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<stdlib.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
int main(int argc,char**argv){
	uint32_t vtxno=atoi(argv[1]);
    nynn_fs fs("192.168.255.115:50000","192.168.255.115:60000");
    uint32_t blkno=nynn_file::headblkno;
    nynn_file f(fs,vtxno);
    cout<<vtxno<<":"<<endl;
    while(blkno!=nynn_file::invalidblkno){
		shared_ptr<Block> blk=f.read(blkno);
        blkno=(blk->getHeader()->getNext)();
    	EdgeContent *ectt=*blk.get();
        uint16_t size=ectt->size();
		cout<<"size:"<<size<<endl;
        for(uint16_t i=0;i<size;i++){
			cout<<ectt->pos(i)->m_sink<<" "<<ectt->pos(i)->m_timestamp<<" "<<ectt->pos(i)->type<<" "<<ectt->pos(i)->topic<<endl;

        }
    }
}
