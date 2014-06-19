#include<sys/socket.h>
#include<netinet/in.h>
#include "nynn_ipc.hpp"
#include<string.h>
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
//	MmapFile m(fpath);
    int fd=getSockfd("192.168.255.118",9999);
/*    for(;;){
		string buff;
        cin>>buff;
        cout<<buff;
        write(fd,buff.c_str(),10);	
        if(buff=="bye") break;
    }
*/  
  /*  int snd_size =1024*8*2*2*2*2;  
    int optlen = sizeof(snd_size); 
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen); */
   // if(err<0) cout<<"buffer error";
    char buff[65536];
    uint32_t n=1600;
    double len=0,tmp;
    long time_pre=getTime();
    while(n>0){
		tmp=write(fd,buff,65536);
        len+=tmp;
        n--;
    }    
    long time_next=getTime();
    cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<len/1024/1024<<"MB throughout:"<<len/1024/1024/(time_next-time_pre)*1000<<"MB/s"<<endl;
    close(fd);
}



