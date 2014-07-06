#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<linuxcpp.hpp>
#include<nynn_mm_edgemanip.hpp>
#include<dirent.h>
#include<errno.h>
#include<time.h>
#include<pthread.h>
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
string suffix;
string dirname;
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
string rescuefile; 
string nowfile("no");
int cirtime;
ofstream ofok,oflog;
fstream fok,flog;


void input_all(SubgraphSet&,int);
void increase(SubgraphSet&,int);

void* heartbeat(void *arg)
{	
	int listenfd,connfd;
    struct sockaddr_in servaddr,c_addr;
        if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0){
		cout<<"hb socket error"<<endl;
        return NULL;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(10001);
    while(::bind(listenfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0)
    {	
		cout<<"hb bind error again"<<endl;
        sleep(2);
    }
	cout<<"bind ok"<<endl;
    if(listen(listenfd,32)<0)
    {   
        cout<<"hb listen error"<<endl;
        return NULL;
    }
    
    socklen_t socklen=sizeof(struct sockaddr);
	cout<<"accept is coming"<<endl;
    connfd=accept(listenfd,(struct sockaddr *)&c_addr,&socklen);
	cout<<"accept ok"<<endl;
	time_t now=time(NULL);
	char buff[10];
	int n;
	while(write(connfd,&now,sizeof(time_t))>0){
		if((n=recv(connfd,buff,10,0))>0){
			sleep(5);
		}
		now=time(NULL);
	}
    close(connfd);
	close(listenfd);
	exit(0);	
}

int main(int argc,char**argv)
{
    string basedir=argv[1];
    SubgraphSet sgs(basedir);
	pthread_t thread;
	pthread_create(&thread,NULL,heartbeat,NULL);
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
    while(::bind(listenfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0)
    {	
		cout<<"bind error again"<<endl;
        sleep(2);
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
		if((connfd<0)&&(errno==EINTR)) continue;
        cout<<"has a connection...."<<endl;
		//if(access("rescue.log",0)!=0){
		
		//get suffix
	    int n=recv(connfd,buff,BUF_SIZE,0);
		if(n>0){
			buff[n]='\0';
			suffix=buff;
			cout<<suffix<<endl;
			write(connfd,"ok",2);
			tail=0;
			flag=0;
			if(strcmp(buff,".data")==0){
				input_all(sgs,connfd);
			}else 
				if(strcmp(buff,".log")==0){
					nowfile="no";
					rescuefile="";
					ofok.close();
					fok.close();
					oflog.close();
					flog.close();
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
	
	//get dirname
	n=recv(connfd,buff,BUF_SIZE,0);
	uint32_t donefiles[100];
	uint32_t donelen=0;
	int isput;
	if(n>0){
		buff[n]='\0';
		dirname=buff;	
	}
		
	//id dir to rescue
	if(access(dirname.c_str(),0)!=0){
		mkdir(dirname.c_str(),S_IRWXU);
		fok.open((dirname+"/ok.log").c_str(),fstream::out|fstream::in|fstream::app);
		write(connfd,"ok",2);		
	}else{
		write(connfd,"no",2);
		recv(connfd,buff,BUF_SIZE,0);
		if(access((dirname+"/ok.log").c_str(),0)!=0){
			donefiles[0]=0;
			donelen=4;
			fok.open((dirname+"/ok.log").c_str(),fstream::out|fstream::in|fstream::app);
		}else{
			donefiles[0]=0;
			donelen=4;
			fok.open((dirname+"/ok.log").c_str());
			fok.seekg(0,fstream::beg);
			uint32_t tmp;
			while(fok>>tmp){
				donefiles[0]++;
				donelen+=4;
				donefiles[donefiles[0]]=tmp;
				
			}
			fok.clear();
			fok.seekp(0,fstream::end);
		}
		write(connfd,donefiles,donelen);
		DIR* dir;
    	struct dirent* dirent;
    	dir=opendir(dirname.c_str());	
		if(dir!=NULL){
            while(1){
            	dirent=readdir(dir);
            	if(dirent==NULL) break;
				if(dirent->d_type!=DT_DIR){
					if(strcmp(dirent->d_name,"ok.log")!=0){
						rescuefile=dirname+"/"+dirent->d_name;
						cout<<"rescuefile:"<<rescuefile<<endl;
					}
				}
			}

        }else{
			cout<<"dir not open"<<endl;
		}
	}

	while((n=recv(connfd,buff,BUF_SIZE,0))>0){
		uint32_t cmp=-1;
		memcpy(all_data,no_done,tail);
        memcpy(all_data+tail,buff,n);
        size=tail+n;
        base=all_data;
		cout<<"size:"<<size<<endl;
        while(1){
			if(flag==0){
                if(size>=4){
                	uint32_t *pv=(uint32_t *)base;
					vtxno=*pv;
					//cout<<vtxno<<endl;
					if(vtxno==cmp){
						cout<<"vtxno:"<<vtxno<<endl;
						flag=3;
						//close old file
						if(strcmp(nowfile.c_str(),"no")!=0){
							cout<<"close old file:";
							if(nowfile!=rescuefile){
								oflog<<"stop"<<endl;
								oflog.close();
							}else{
								string stop;
								if(flog>>stop){
								}
								else{
									flog.clear();
									flog.seekg(0,fstream::end);
									flog<<"stop"<<endl;
								}
								flog.close();
							}
							char tmp[50];
							strcpy(tmp,nowfile.c_str());
							cout<<tmp<<endl;
							int position[2];
							char delims[2]={'/','.'};
							for(int i=0;i<2;i++){
								for(int m=0;m<nowfile.length();m++){
									if(tmp[m]==delims[i]){
										position[i]=m+1;
										break;
									}
								}
							}
							char tmp1[50];
							memcpy(tmp1,tmp+position[0],position[1]-position[0]-1);
							tmp1[position[1]-position[0]-1]='\0';
							//cout<<"ok.log:"<<tmp1<<endl;
							fok<<tmp1<<endl;
							//remove(nowfile.c_str());
						}
						if(size<8){
							tail=size;
							memcpy(no_done,base,tail);
							break;
						}
						continue;
					}
                    if(nowfile==rescuefile){
						if(isput==0){
							string begin;
							uint32_t vtx,blk;
							size_t sizeno;
							if(flog>>begin>>vtx>>blk>>sizeno){
								string end;
								if(flog>>end){
									//isput is still 0
									pv++;
                                    base=pv;
                                    size-=4;
                                    // cout<<"new:"<<vtxno<<endl;
									cout<<"ignore"<<endl;
                                    flag=1;
                                    continue;
								}else{
									//add rescue
									flog.clear();
									flog.seekp(0,fstream::end);
									cout<<"rescue...."<<vtx<<endl;
									edgemanip.pop_edges_until(vtx,blk,sizeno);
									isput=1;
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
										
								}
							}
							else{
								flog.clear();
                                flog.seekp(0,fstream::end);
								isput=1;
								continue;	
							}
						}
						else{
							uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
    		                if (unlikely(!sgs.exists(sgkey))){
                           		sgs.createSubgraph(sgkey);
                            }
							uint32_t pblk;
							size_t psize;
							edgemanip.current_tail(vtxno,&pblk,&psize);
							flog<<"begin "<<vtxno<<" "<<pblk<<" "<<psize<<endl;
                            pv++;
                            base=pv;
                            size-=4;
                             // cout<<"new:"<<vtxno<<endl;
                            flag=1;
                            continue;	
						}
					}
					else{
						uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
                    	if (unlikely(!sgs.exists(sgkey))){
                    		sgs.createSubgraph(sgkey);
                    	}
            	        uint32_t pblk;
						size_t psize;
						edgemanip.current_tail(vtxno,&pblk,&psize);
						oflog<<"begin "<<vtxno<<" "<<pblk<<" "<<psize<<endl;
                		pv++;
                		base=pv;
                		size-=4;
                   		// cout<<"new:"<<vtxno<<endl;
                    	flag=1;
                    	continue;
					}
                }else{
					tail=size;
                    memcpy(no_done,base,tail);
                    flag=0;
                    break;
                }
            }
			if(flag==3){
				uint32_t *pover=(uint32_t *)base;
				uint32_t over=*(pover+1);
				cout<<"over:"<<over<<endl;
				if(over==0){
					tail=0;
					flag=0;
					nowfile="no";
					rescuefile="";
					write(connfd,"success",7);
				    break;
				}
				else{
					if(size<50){
						tail=size;
                        memcpy(no_done,base,tail);
                        break;
					}
				}
				cout<<"no end"<<endl;
				uint32_t *pnew=(uint32_t *)base;
				pnew++;
				uint32_t len=*pnew;
				cout<<"len:"<<len<<endl;
				pnew++;
				string name((char *)pnew,len);
				cout<<"new file:"<<name<<endl;
				nowfile=name;
				char *ptmp=(char*)pnew;
				ptmp+=len;
				base=ptmp;
				size-=(4+4+len);
				flag=0;
                //add
				if(name==rescuefile){
					isput=0;
					cout<<"rescuefile return"<<endl;
					flog.open(name.c_str());
					flog.seekg(0,fstream::beg);
					string start;
					if(flog>>start){
						cout<<"log:"<<start<<endl;
					}else{
						flog.clear();
						flog.seekp(0,fstream::end);
						flog<<"start"<<endl;
						isput=1;
					}
				}
				else{
					isput=1;
					string tmp=name;
					oflog.open(tmp.c_str());
					oflog<<"start"<<endl;					
				}
				continue;		
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
						if(isput)
							edgemanip.push_edges(vtxno,pe,pe+edge_num);
                    	size-=(edge_num*e_size);
						base=pe+edge_num;
						edge_num=0;
						flag=0;			
						if(nowfile==rescuefile){
							if(isput)
								flog<<"end"<<endl;
						}else{
							oflog<<"end"<<endl;	
						}
						continue;
					}else{
						if(edge_full==0){
							tail=size;
							memcpy(no_done,base,tail);
                            flag=2;
                            break;
						}else{
							pe=(Edge *)base;
							if(isput)
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
					if(nowfile==rescuefile){
						if(isput)
    				    	flog<<"end"<<endl;
    	            }else{
                        oflog<<"end"<<endl;
                    }
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
    n=recv(connfd,buff,BUF_SIZE,0);
	if(n>0){
		buff[n]='\0';
		dirname=buff;	
	}
		
	write(connfd,"ok",2);
	uint32_t cmp=-1;

    while((n=recv(connfd,buff,BUF_SIZE,0))>0){
       /* buff[n]='\0';
		cout<<buff<<endl;
		continue;*/
        memcpy(all_data,no_done,tail);
        memcpy(all_data+tail,buff,n);
        size=tail+n;
        base=all_data;
		cout<<"size:"<<size<<endl;
        while(1){
			if(flag==0){
                if(size>=4){
                	uint32_t *pv=(uint32_t *)base;
					vtxno=*pv;
					cout<<"here"<<vtxno<<endl;
					if(vtxno==cmp){
						cout<<"-1"<<endl;
						flag=3;
						if(size<8){
							tail=size;
							memcpy(no_done,base,tail);
							break;
						}
						continue;
					}
				//	cout<<vtxno<<endl;
                    uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
                    if (unlikely(!sgs.exists(sgkey))){
                    	sgs.createSubgraph(sgkey);
                    }
					sgs.push(vtxno,NULL);
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
			if(flag==3){
				uint32_t *pover=(uint32_t *)base;
				uint32_t over=*(pover+1);
				if(over==0){
					tail=0;
					flag=0;
					write(connfd,"success",7);
					break;
				}else{
					if(size<50){
						tail=size;
						memcpy(no_done,base,tail);
						break;
					}
				}
				cout<<"no end"<<endl;
				uint32_t *pnew=(uint32_t *)base;
				pnew++;
				uint32_t len=*pnew;
				pnew++;
				string name((char*)pnew,len);
				char *ptmp=(char*)pnew;
				ptmp+=len;
				base=ptmp;
				size-=(4+4+len);
				flag=0;
				continue;
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
   
