#include<sys/socket.h>
#include<netinet/in.h>
#include "nynn_ipc.hpp"
#include<string.h>
#include<dirent.h>
#include<stdlib.h>
#include<pthread.h>
#define BUF_SIZE 65536
#define IP_NUM 1
char io_tmp[1024];
//string ips[IP_NUM]={"192.168.255.115","192.168.255.117","192.168.255.118"};
string ips[IP_NUM]={"192.168.255.115"};
struct balance{
	string ip;
    int num;
    int sgs[1024];
};
struct balance bs[IP_NUM];
struct balance bs_thread[IP_NUM];
unsigned int files[2048];
int file_len;
//char suffix[10];
//int suf_len;
//char prefix[256];
//int pre_len;
string prefix;
string suffix;
void init()
{
	if(access("balance.cfg",0)!=0){
		ofstream fout("balance.cfg");
        for(int i=0;i<IP_NUM;i++)
			fout<<ips[i]<<":"<<0<<'\n';
		fout.close();
    }
}
int get_min()
{
	int min;
	for(int i=0;i<IP_NUM;i++){
		if(i==0) 
			min=0;
		else 
 			if(bs[i].num<bs[min].num) min=i;      		
	}
    return min;
}
int is_in(int file)
{
	for(int i=0;i<IP_NUM;i++)
		for(int j=0;j<bs[i].num;j++)
			if(file==bs[i].sgs[j]) return i;	
    return -1;
}
void distribute(unsigned int *files,int size){
	for(int i=0;i<IP_NUM;i++){
		bs_thread[i].ip=bs[i].ip;
		bs_thread[i].num=0;	
	}
    int n;
    int min;
	for(int i=0;i<size;i++){
		if((n=is_in(files[i]))!=-1){
			bs_thread[n].sgs[bs_thread[n].num]=files[i];
            bs_thread[n].num++;
    	}else{
			min=get_min();
			bs_thread[min].sgs[bs_thread[min].num]=files[i];
            bs_thread[min].num++;					
			bs[min].sgs[bs[min].num]=files[i];
            bs[min].num++;
		}	
	}
}
void input_balance(int i,char *buf)
{
	char delims[]=":";
    char *result=NULL;
    result=strtok(buf,delims);
    bs[i].ip=result;
    result=strtok(NULL,delims);
    bs[i].num=atoi(result);
    for(int n=0;n<bs[i].num;n++){
		result=strtok(NULL,delims);
		bs[i].sgs[n]=atoi(result);
    }
}
void read_balance()
{
	ifstream fin;
    fin.open("balance.cfg");
    int i=0;
    while(fin.getline(io_tmp,sizeof(io_tmp))){
    	input_balance(i++,io_tmp);
    }
    fin.close();
}
void write_balance()
{
	ofstream fout("balance.cfg");
    for(int i=0;i<IP_NUM;i++){
    	fout<<bs[i].ip<<":"<<bs[i].num;
        for(int j=0;j<bs[i].num;j++)
			fout<<":"<<bs[i].sgs[j];
        fout<<'\n';
    }
    fout.close();
}
void print_bs()
{
	for(int i=0;i<IP_NUM;i++){
		cout<<bs[i].ip<<" "<<bs[i].num<<":";
        for(int j=0;j<bs[i].num;j++){
			cout<<bs[i].sgs[j]<<" ";
		}
        cout<<endl;
	}
}
void print_bs_t()
{
    for(int i=0;i<IP_NUM;i++){
         cout<<bs_thread[i].ip<<" "<<bs_thread[i].num<<":";
         for(int j=0;j<bs_thread[i].num;j++){
 	        cout<<bs_thread[i].sgs[j]<<" ";
   		 }
    	cout<<endl;
    }
}
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
int parse_dir(string dir_name)
{
	DIR* dir;
	struct dirent* dirent;
    dir=opendir(dir_name.c_str());
	prefix=dir_name+"/";
    char file[256];
    char delims[]=".";
    char *result;
    file_len=0;
    int flag=1;
	if(dir!=NULL){
		while(1){
			dirent=readdir(dir);
			if(dirent==NULL) break;
			if(dirent->d_type!=DT_DIR){
		   	//	cout<<strlen(dirent->d_name)<<endl;
				memcpy(file,dirent->d_name,dirent->d_reclen);
			//	cout<<file<<endl;
     	    	result=strtok(file,delims);
			//	cout<<result<<endl;
				files[file_len]=atoi(result);
				file_len++;      
                if(flag==1){
					result=strtok(NULL,delims);
				/*	char tmp[2]=".";
					memcpy(suffix,tmp,1);
					memcpy(suffix+1,result,strlen(result));
					suf_len=strlen(result)+1;*/
					suffix=".";
                    suffix=suffix+result;
           //       cout<<suf_len<<endl;
					flag=0;
				}      
			}
		}	
        return 1;
	}else{
		cout<<"dir not exist"<<endl;
		return -1;
	}
}

string get_filename(int i)
{
//	char *p=itoa(i);
    char p[256];
    sprintf(p,"%d",i);
	string filename=prefix+p+suffix;
//	cout<<filename<<" "<<filename.length()<<endl;
	return filename;
  
}
void io_func(string,int);
void* my_thread(void *arg)
{
	int *n=(int *)arg;
	string ip=bs_thread[*n].ip;
    char buff[10];
//	cout<<ip<<endl;
    int fd=getSockfd(ip,9999);
	write(fd,suffix.c_str(),suffix.length());
    recv(fd,buff,10,0);    
	for(int i=0;i<bs_thread[*n].num;i++){
		string fpath=get_filename(bs_thread[*n].sgs[i]);
        cout<<ip<<" "<<fpath<<endl;
		io_func(fpath,fd);
	}    
    close(fd);	
}

void io_func(string fpath,int fd)
{
    MmapFile m(fpath);
    //int fd=getSockfd(ip,9999);
    char buff[BUF_SIZE];
    double len=0,tmp;
    char* base=(char *)m.getBaseAddress();
   
/*	write(fd,fpath.c_str(),strlen(fpath.c_str()));
    return;*/


    size_t length=m.getLength();
    size_t tail=length%BUF_SIZE;
    size_t cycle=(tail!=0)?length/BUF_SIZE+1:length/BUF_SIZE;
    cout<<length<<" "<<cycle<<" "<<tail<<endl;    
   // long time_pre=getTime();
    for(int n=0;n<cycle;n++){
        if((n==(cycle-1))&&(tail!=0)){ 
        	tmp=write(fd,base+n*BUF_SIZE,tail);
			if(tmp!=tail){
				cout<<"write error"<<endl;
				return;
			}
		}    
		else{
        	tmp=write(fd,base+n*BUF_SIZE,BUF_SIZE);
            if(tmp!=BUF_SIZE){
                cout<<"write error"<<endl;
            	return;
            }
		}
        len+=tmp;
    }    
   // long time_next=getTime();
   // cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<len/1024/1024<<"MB throughout:"<<len/1024/1024/(time_next-time_pre)*1000<<"MB/s"<<endl;
   // close(fd);
}
void zms_client(string dir_name)
{
	pthread_t threads[IP_NUM];
    int args[IP_NUM];
	init();
    read_balance();	
	parse_dir(dir_name);
	distribute(files,file_len);
	print_bs();
    print_bs_t();
	write_balance();
	for(int n=0;n<IP_NUM;n++){
    	args[n]=n;
        pthread_create(&threads[n],NULL,my_thread,(void *)&args[n]);
    }
    for(int n=0;n<IP_NUM;n++){
        pthread_join(threads[n],NULL);
    }
}


