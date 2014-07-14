#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<pthread.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
#define THREADNUM 32
int threadNum;
uint32_t all_vtxno;
int vtxNum;
double *counts;
nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
} 
void*  my_thread(void *arg){
    int *n=(int *)arg;
    uint32_t vtxno=all_vtxno;
    uint32_t vtxend=vtxno+vtxNum;
 	Block blk;
	CharContent *cctt=blk;
    uint32_t blkno=nynn_file::headblkno;
    counts[*n]=0;
    while(vtxno<vtxend){
         uint32_t blkno=nynn_file::headblkno;
         nynn_file f(fs,vtxno);
         while(blkno!=nynn_file::invalidblkno){
            if(!f.read(blkno,&blk)) break;
            blkno=(blk.getHeader()->getNext)();
            string line(cctt->begin(),cctt->end());
            counts[*n]+=line.size();
         }
         vtxno++;
    } 
}
int main(int argc,char**argv){
    int i=0,j;
    threadNum=atoi(argv[1]);
    all_vtxno=atoi(argv[2]);
    vtxNum=atoi(argv[3]);
    double countAll=0;
    pthread_t threads[threadNum];
    counts=new double[threadNum];
    int args[threadNum];
    long time_pre=getTime();
    for(int n=0;n<threadNum;n++){
        args[n]=n;
        pthread_create(&threads[n],NULL,my_thread,(void *)&args[n]);
    }
    for(int n=0;n<threadNum;n++){
    	pthread_join(threads[n],NULL);
    }
    long time_next=getTime();
    for(int i=0;i<threadNum;i++){
         countAll+=counts[i];
    }
    cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<countAll<<"B throughout:"<<countAll/1024/1024/(time_next-time_pre)*1000<<"MB/s";
    
}
