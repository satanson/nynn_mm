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
    nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
    uint32_t blkno=nynn_file::headblkno;
    nynn_file f(fs,vtxno,false);
    Block blk;
    EdgeContent *ectt=blk;
    cout<<vtxno<<":"<<endl;
    while(blkno!=nynn_file::invalidblkno){
        if(!f.read(blkno,&blk)) break;
        blkno=(blk.getHeader()->getNext)();
        uint16_t size=ectt->size();
        for(uint16_t i=0;i<size;i++){
			cout<<ectt->pos(i)->m_sink<<" "<<ectt->pos(i)->m_weight.m_fval<<" "<<ectt->pos(i)->m_timestamp<<endl;
        }
    }
}
