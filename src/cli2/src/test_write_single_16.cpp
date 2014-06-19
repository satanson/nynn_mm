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
  
    string data;
    data.resize(CharContent::CONTENT_CAPACITY);
    uint32_t vtxno;
	nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
	Block blk;
	CharContent *cctt=blk;
    int i=0,j;
    long time_pre=getTime();
	while(i<16){
        vtxno=0;
        while(vtxno<64){
            j=0;
        	nynn_file f(fs,vtxno,true);
            while(j<4){
				cctt->resize(data.size());
				std::copy(data.begin(),data.end(),cctt->begin());
				f.push(&blk);
                j++;
            } 
        	vtxno++;
       }
       i++;
   }
   long time_next=getTime();
   cout<<time_next-time_pre; 
}
