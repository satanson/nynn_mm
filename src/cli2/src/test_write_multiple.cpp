#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<pthread.h>
#include<stdlib.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
int blkNum;
int threadNum;
int vtxNum;
double *counts;
long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
} 
void*  my_thread(void *arg){
    int *n=(int *)arg;
    int begin=(*n)*1024;
    int end=vtxNum+(*n)*1024;
 	Block blk;
	CharContent *cctt=blk; 
    string data;
    data.resize(CharContent::CONTENT_CAPACITY);
    counts[*n]=0;
    for(uint32_t vtxno=begin;vtxno<end;vtxno++){
        int j=0;
        nynn_file f(fs,vtxno);
        while(j<blkNum){
			cctt->resize(data.size());
			std::copy(data.begin(),data.end(),cctt->begin());
			f.push(&blk);
            counts[*n]+=data.size();
            j++;
        }
    }    
       
}
int main(int argc,char**argv){
    int i=0,j;
    threadNum=atoi(argv[1]);
    vtxNum=atoi(argv[2]);
    blkNum=atoi(argv[3]);
    counts=new double[threadNum];
    pthread_t threads[threadNum];
    int args[threadNum];
    double countAll=0;
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
