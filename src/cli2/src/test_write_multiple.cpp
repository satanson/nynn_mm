#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<pthread.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
#define VTXNUM 1024
#define THREADNUM 16
nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
} 
void*  my_thread(void *arg){
    int *n=(int *)arg;
    int begin=(*n)*VTXNUM;
    int end=VTXNUM+(*n)*VTXNUM;
 	Block blk;
	CharContent *cctt=blk; 
    string data;
    data.resize(CharContent::CONTENT_CAPACITY);
    for(uint32_t vtxno=begin;vtxno<end;vtxno++){
        int j=0;
        nynn_file f(fs,vtxno,true);
        while(j<16){
			cctt->resize(data.size());
			std::copy(data.begin(),data.end(),cctt->begin());
			f.push(&blk);
            j++;
        }
        vtxno++;
        cout<<(*n)<<"->"<<vtxno<<"over ";
    }    
       
}
int main(int argc,char**argv){
    int i=0,j;
    pthread_t threads[THREADNUM];
    int args[THREADNUM];
    for(int n=0;n<THREADNUM;n++){
		args[n]=n;
        pthread_create(&threads[n],NULL,my_thread,(void *)&args[n]);
    }
    long time_pre=getTime();
    for(int n=0;n<THREADNUM;n++){
    	pthread_join(threads[n],NULL);
    }
    long time_next=getTime();
    cout<<time_next-time_pre; 
}
