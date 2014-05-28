#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

//retun current time "ms"
long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main(int argc,char**argv){
    uint32_t vtxno=4096;
	nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
	Block blk;
	CharContent *cctt=blk;
    int i=0,j;
    long time_pre=getTime();
    while(vtxno<5119){
   	    uint32_t blkno=nynn_file::headblkno;
        nynn_file f(fs,vtxno,false);
        while(blkno!=nynn_file::invalidblkno){
			if(!f.read(blkno,&blk)) break;
            blkno=(blk.getHeader()->getNext)();
            string line(cctt->begin(),cctt->end());
        }
        vtxno++;
   }
   long time_next=getTime();
   cout<<time_next-time_pre; 
}
