#include<nynn_mm_config.hpp>
#include<sys/time.h>
long getTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main(int argc,char**argv)
{
    string basedir=argv[1];
	string fpath=argv[2];
    SubgraphSet sgs(basedir);
    MmapFile m(fpath);
    void* base=m.getBaseAddress();
    char *pc_end=(char *)base;
    void *pv_end;
    size_t length=m.getLength();
    pc_end+=length;
    pv_end=pc_end;
    Block blk;
    EdgeContent *ectt=blk;
    double counts=0;
    double icounts=0;
    double col=0;
    long time_pre=getTime();
    while(base!=pv_end){
        col++;
	    cout<<base<<"-"<<pv_end<<" "<<length<<" "<<col<<endl;
 		uint32_t *p1=(uint32_t*)base;
        uint32_t vtxno=*p1;
        if (unlikely(!sgs.vtx_exists(vtxno))){
       		uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
            sgs.createSubgraph(sgkey);
        }
        base=++p1;
        base=p1+1;
        Edge* pe=(Edge*)base;
        uint32_t i=0;
        uint16_t size=0;
        icounts+=8;
        while(i<*p1){
           if(size<EdgeContent::CONTENT_CAPACITY){
				ectt->resize(++size);
                Edge *tmp=ectt->pos(ectt->size()-1);
                tmp->m_sink=pe->m_sink;
                tmp->m_weight.m_fval=pe->m_weight.m_fval;
                tmp->m_timestamp=pe->m_timestamp;    
                counts+=20;
           } 
           if(size==EdgeContent::CONTENT_CAPACITY){
                sgs.push(vtxno,&blk); 
                size=0;
           }
       	   base=++pe;
           i++;
	   }
	   if(size>0) sgs.push(vtxno,&blk); 
   }   
   long time_next=getTime();
   cout<<(icounts+counts)/1024/1024<<endl;
   cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<counts/1024/1024<<"MB throughout:"<<counts/1024/1024/(time_next-time_pre)*1000<<"MB/s";
}
