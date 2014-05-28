#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<sys/time.h>
#include<stdlib.h>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

long getTime()
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
}

int main(int argc,char**argv){
    string data;
    data.resize(CharContent::CONTENT_CAPACITY);
    double throughout=0; 
    uint32_t vtxno=atoi(argv[1]);
    int vtxNum=atoi(argv[2]);
    int blkNum=atoi(argv[3]);
    int vtxend=vtxno+vtxNum;
    int reply=atoi(argv[4]);
	nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
	Block blk;
	CharContent *cctt=blk;
    int i=0,j;
    long time_pre=getTime();
	while(i<reply){
        vtxno=atoi(argv[1]);
        while(vtxno<vtxend){
            j=0;
        	nynn_file f(fs,vtxno,true);
            while(j<blkNum){
				cctt->resize(data.size());
				std::copy(data.begin(),data.end(),cctt->begin());
				f.push(&blk);
                throughout+=data.size();
                j++;
            }
        	vtxno++;
       }
       i++;
   }
   long time_next=getTime();
   cout<<"time:"<<time_next-time_pre<<" datasize:"<<throughout<<"B throurhout:"<<throughout/1024/1024/(time_next-time_pre)/1000<<"MB/s"; 
}
