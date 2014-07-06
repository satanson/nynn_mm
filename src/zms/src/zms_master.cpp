#include<iostream>
#include<sys/socket.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/time.h>
#include<string>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
using namespace std;
#define NUM 1
string ips[NUM]={"192.168.255.115"};
int flags[NUM];
int fds[NUM];
pthread_t threads[NUM];
time_t times[NUM];
pid_t pids[NUM];
int initflags[NUM];
void* heartbeat(void *arg)
{
	int n=*(int*)arg;
	char buff[10];
	struct sockaddr_in servaddr;
    if((fds[n]=socket(AF_INET,SOCK_STREAM,0))<0){
        cout<<"socket error"<<endl;
        goto recycle;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(10001);
    if(inet_pton(AF_INET,ips[n].c_str(),&servaddr.sin_addr)<0){
        cout<<"inte_pton error for "<<ips[n]<<endl;
        goto recycle;
    }
    while(connect(fds[n],(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0){
        cout<<ips[n]<<":"<<strerror(errno)<<"  connect again..."<<endl;
        sleep(5);
        close(fds[n]);
        if((fds[n]=socket(AF_INET,SOCK_STREAM,0))<0){
           cout<<"socket error"<<endl;
           goto recycle;
        }
    }
	cout<<"haha okok"<<endl;	
	initflags[n]=1;
	int m;	
	while((m=recv(fds[n],buff,10,0))>0){
		times[n]=*(time_t *)buff;
		write(fds[n],"ok",2);
	}
	recycle:
	while(1);
}

int main()
{
	int args[NUM];
	//init
	for(int i=0;i<NUM;i++){
		args[i]=i;
		initflags[i]=0;
		string comm="ssh bsp@"+ips[i]+" \"/home/bsp/programer/nynn_mm/src/zms_server1 /home/bsp/programer/nynn/graph  >>/home/bsp/programer/nynn_mm/src/logfile &\"";
		system(comm.c_str());		
		pthread_create(&threads[i],NULL,heartbeat,(void *)&args[i]);
	}
	for(int i=0;i<NUM;i++){
		times[i]=time(NULL);
	}
	while(1){	
		time_t now=time(NULL);
		for(int i=0;i<NUM;i++){
            if((now-times[i])>15){
				if(initflags[i]==1){
					cout<<"chaole"<<endl;
					pthread_cancel(threads[i]);
					close(fds[i]);	
					initflags[i]=0;
					string comm="ssh bsp@"+ips[i]+" \"/home/bsp/programer/nynn_mm/src/zms_server1 /home/bsp/programer/nynn/graph  >>/home/bsp/programer/nynn_mm/src/logfile &\"";
     	        	system(comm.c_str());       
                	pthread_create(&threads[i],NULL,heartbeat,(void *)&args[i]);
				}
			}
		}
		sleep(20);
	}
	
}


