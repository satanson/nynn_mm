#include<sys/socket.h>
#include<netinet/in.h>
#include "nynn_ipc.hpp"
#include<string.h>
#define BUF_SIZE 65536
long getTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
}
int getSockfd(string ip,uint16_t port)
{
    int sockfd;
	struct sockaddr_in servaddr;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
    	cout<<"socket error"<<endl;
        return -1;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    if(inet_pton(AF_INET,ip.c_str(),&servaddr.sin_addr)<0){
		cout<<"inte_pton error for "<<ip<<endl;
        return -2;
    } 
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0){
		cout<<ip<<" connect error"<<endl;
        return -3;
    }
    return sockfd;
        
}
int zms_client(string fpath)
{
    MmapFile m(fpath);
    int fd=getSockfd("192.168.255.118",9999);
    char buff[BUF_SIZE];
    double len=0,tmp;
    char* base=(char *)m.getBaseAddress();
    size_t length=m.getLength();
    size_t tail=length%BUF_SIZE;
    size_t cycle=(tail!=0)?length/BUF_SIZE+1:length/BUF_SIZE;
    cout<<length<<" "<<cycle<<" "<<tail<<endl;    
    long time_pre=getTime();
    for(int n=0;n<cycle;n++){
        if((n==(cycle-1))&&(tail!=0)) 
        	tmp=write(fd,base+n*BUF_SIZE,tail);
            
		else
        	tmp=write(fd,base+n*BUF_SIZE,BUF_SIZE);
        len+=tmp;
    }    
    long time_next=getTime();
    cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<len/1024/1024<<"MB throughout:"<<len/1024/1024/(time_next-time_pre)*1000<<"MB/s"<<endl;
    close(fd);
}



