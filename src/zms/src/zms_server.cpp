#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<linuxcpp.hpp>
#include<nynn_mm_edgemanip.hpp>
#include "nynn_mm_config.hpp"
#define BUF_SIZE 65536
typedef edge_manip_t<Edge> EdgeManip;
int n;
double len=0;
int flag=0;
int tail=0;
uint32_t size;
char no_done[64];
char all_data[BUF_SIZE+64];
Edge* pe;
int e_size=sizeof(struct Edge);
//cout<<"edge size:"<<e_size<<endl;
Block blk;
EdgeContent *ectt=blk;
int blk_size;   
uint32_t vtxno;
uint32_t edge_num;
void *base;
char buff[BUF_SIZE];

void input_all(SubgraphSet&,int);
void increase(SubgraphSet&,int);

int main(int argc,char**argv)
{
    string basedir=argv[1];
    SubgraphSet sgs(basedir);
	int listenfd,connfd;
    struct sockaddr_in servaddr,c_addr;
        if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0){
		cout<<"socket error"<<endl;
        return -1;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(9999);
    if(::bind(listenfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0)
    {	
		cout<<"bind error"<<endl;
        return -2;
    }
    if(listen(listenfd,32)<0)
    {   
        cout<<"bind error"<<endl;
        return -3;
    }
    
    socklen_t socklen=sizeof(struct sockaddr);
    
    while(1){
        cout<<"listening...."<<endl; 		
    	connfd=accept(listenfd,(struct sockaddr *)&c_addr,&socklen);
        cout<<"has a connection...."<<endl;
	    int n=recv(connfd,buff,BUF_SIZE,0);
		if(n>0){
			buff[n]='\0';
			cout<<buff<<endl;
			write(connfd,"ok",2);
			if(strcmp(buff,".data")==0){
				input_all(sgs,connfd);
			}else 
				if(strcmp(buff,".log")==0){
					increase(sgs,connfd);
				}
		}		
		close(connfd);
    }
    cout<<"sum:"<<len<<endl;
    close(listenfd);
    return 1;     
}

void increase(SubgraphSet &sgs,int connfd)
{
    uint32_t edge_full;
	EdgeManip edgemanip(sgs);
	while((n=recv(connfd,buff,BUF_SIZE,0))>0){
		memcpy(all_data,no_done,tail);
        memcpy(all_data+tail,buff,n);
        size=tail+n;
        base=all_data;
        while(1){
			if(flag==0){
                if(size>=4){
                	uint32_t *pv=(uint32_t *)base;
					vtxno=*pv;
					uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
                    if (unlikely(!sgs.exists(sgkey))){
                    	sgs.createSubgraph(sgkey);
                    }
                	pv++;
                	base=pv;
                	size-=4;
                   // cout<<"new:"<<vtxno<<endl;
                    flag=1;
                    continue;
                }else{
					tail=size;
                    memcpy(no_done,base,tail);
                    flag=0;
                    break;
                }
            }
            if(flag==1){    			
                if(size>=4){
                	uint32_t *pnum=(uint32_t *)base;	
                	edge_num=*pnum;
                	pnum++;
                	base=pnum;
                    size-=4;
                   // cout<<"num:"<<edge_num<<endl;
                    flag=2;
                    blk_size=0;
                    continue;
				}else{
					tail=size;
					memcpy(no_done,base,tail);
                    flag=1;	
                    break;
				}                    
            }
            if(flag==2){
				edge_full=size/e_size;
				if(edge_num>0){
			    	if(edge_full>=edge_num){
						pe=(Edge *)base;
						edgemanip.push_edges(vtxno,pe,pe+edge_num);
                    	size-=(edge_num*e_size);
						base=pe+edge_num;
						edge_num=0;
						flag=0;				
						continue;
					}else{
						if(edge_full==0){
							tail=size;
							memcpy(no_done,base,tail);
                            flag=2;
                            break;
						}else{
							pe=(Edge *)base;
                            edgemanip.push_edges(vtxno,pe,pe+edge_full);
                            size-=(edge_full*e_size);
                            tail=size;
                            memcpy(no_done,base,tail);
                            edge_num-=edge_full;
                            flag=2;
                            break;
						}
					}
                }else{
			   		flag=0;
					continue;	
			    }	
			}
		}
		len+=n;
	}  
    if(n==0){
        cout<<"increase over"<<endl;
    }
}

void input_all(SubgraphSet &sgs,int connfd)
{
    while((n=recv(connfd,buff,BUF_SIZE,0))>0){
       /* buff[n]='\0';
		cout<<buff<<endl;
		continue;*/
        memcpy(all_data,no_done,tail);
        memcpy(all_data+tail,buff,n);
        size=tail+n;
        base=all_data;
        while(1){
			if(flag==0){
                if(size>=4){
                	uint32_t *pv=(uint32_t *)base;
					vtxno=*pv;
				//	cout<<vtxno<<endl;
                    uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
                    if (unlikely(!sgs.exists(sgkey))){
                    	sgs.createSubgraph(sgkey);
                    }
                	pv++;
                	base=pv;
                	size-=4;
               //   cout<<"new:"<<vtxno<<endl;
                    flag=1;
                    continue;
                }else{
					tail=size;
                    memcpy(no_done,base,tail);
                    flag=0;
                    break;
                }
            }
            if(flag==1){    			
                if(size>=4){
                	uint32_t *pnum=(uint32_t *)base;	
                	edge_num=*pnum;
                	pnum++;
                	base=pnum;
                    size-=4;
                   // cout<<"num:"<<edge_num<<endl;
                    flag=2;
                    blk_size=0;
                    continue;
				}else{
					tail=size;
					memcpy(no_done,base,tail);
                    flag=1;	
                    break;
				}                    
            }
            if(flag==2){
                while((size>=e_size)&&(edge_num>0)){
					pe=(Edge *)base;
                   // cout<<pe->m_sink<<" "<<pe->m_weight.m_fval<<" "<<pe->m_timestamp<<endl;
                    if(blk_size<EdgeContent::CONTENT_CAPACITY){
                    	ectt->resize(++blk_size);
                    	Edge *tmp=ectt->pos(ectt->size()-1);
                    	tmp->m_sink=pe->m_sink;
                    	tmp->m_weight.m_fval=pe->m_weight.m_fval;
                    	tmp->m_timestamp=pe->m_timestamp;
                    }
                    if(blk_size==EdgeContent::CONTENT_CAPACITY){
                        sgs.push(vtxno,&blk);
                        blk_size=0;
                    }
                    pe++;
                    base=pe;
                    size-=e_size; 
                    edge_num--;
                }
                if(edge_num==0){
					flag=0;
                    if(blk_size>0) sgs.push(vtxno,&blk);
                    continue;
                }
                if(edge_num>0){
					tail=size;
                    memcpy(no_done,base,tail);
                    flag=2;
                    break;
                }
           }
           
        }
        len+=n;
     
    }   
    if(n==0){
        cout<<"input over"<<endl;
    }
}
   
