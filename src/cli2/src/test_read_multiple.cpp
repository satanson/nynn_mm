#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<pthread.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
#define THREADNUM 32
nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
} 
void*  my_thread(void *arg){
    uint32_t vtxno=0;
 	Block blk;
	CharContent *cctt=blk;
    uint32_t blkno=nynn_file::headblkno;
    while(vtxno<1024){
         uint32_t blkno=nynn_file::headblkno;
         nynn_file f(fs,vtxno,false);
         while(blkno!=nynn_file::invalidblkno){
            if(!f.read(blkno,&blk)) break;
            blkno=(blk.getHeader()->getNext)();
            string line(cctt->begin(),cctt->end());
         }
         vtxno++;
    } 
}
int main(int argc,char**argv){
    int i=0,j;
    pthread_t threads[THREADNUM];
    long time_pre=getTime();
    for(int n=0;n<THREADNUM;n++)
        pthread_create(&threads[n],NULL,my_thread,NULL);
    for(int n=0;n<THREADNUM;n++){
    	pthread_join(threads[n],NULL);
    }
    long time_next=getTime();
    cout<<time_next-time_pre; 
}
