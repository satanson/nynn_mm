#include<sys/socket.h>
#include<netinet/in.h>
#include "nynn_ipc.hpp"
#include<string.h>
#include<dirent.h>
#include<stdlib.h>
#include<pthread.h>
#include<errno.h>
#define BUF_SIZE 65536
#define IP_NUM 2
char io_tmp[1024];
string ips[IP_NUM]={"192.168.255.117","192.168.255.118"};
//string ips[IP_NUM]={"192.168.255.115"};
struct balance{
	string ip;
    int num;
    int sgs[1024];
};
struct balance bs[IP_NUM];
struct balance bs_thread[IP_NUM];
int success[IP_NUM];
unsigned int files[2048];
int file_len;
//char suffix[10];
//int suf_len;
//char prefix[256];
//int pre_len;
string prefix;
string suffix;
string dirname;
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
	cout<<"all balance:"<<endl;
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
	cout<<"now balance:"<<endl;
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
        return -1;
    } 
    while(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0){
		cout<<ip<<":"<<strerror(errno)<<"  connect again..."<<endl;
        sleep(5);
     	close(sockfd);
     	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
           cout<<"socket error"<<endl;
           return -1;
     	}
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
int io_func(string,int);
void* my_thread(void *arg)
{
	int *n=(int *)arg;
	string ip=bs_thread[*n].ip;
	int number_tmp;
	success[*n]=0;
    char buff[5200];
//	cout<<ip<<endl;
    int fd=getSockfd(ip,9999);
	if(fd==-1)	return NULL;
	number_tmp=write(fd,suffix.c_str(),suffix.length());
	if(number_tmp<suffix.length())	return NULL;
    number_tmp=recv(fd,buff,100,0);  
	if(number_tmp<=0) return NULL;
	number_tmp=write(fd,dirname.c_str(),dirname.length());
	if(number_tmp<dirname.length())  return NULL;
	int m=recv(fd,buff,100,0);
	if(m<=0) return NULL;
	buff[m]='\0';
	uint32_t donefiles[100];
	uint32_t donelen=0;
	if(strcmp(buff,"ok")==0){
		cout<<"good dir"<<endl;
	}else{
		cout<<"bad dir"<<endl;
		number_tmp=write(fd,"file",4);
		if(number_tmp<4) return NULL;
		number_tmp=recv(fd,buff,5200,0);
		if(number_tmp<=0) return NULL;
		uint32_t *p=(uint32_t *)buff;
		donelen=*p;
		for(int i=0;i<donelen;i++){
			p++;
			donefiles[i]=*p;
			cout<<"file:"<<*p<<endl;
		}	
		
	}
	for(int i=0;i<bs_thread[*n].num;i++){
		int isflag=1;
		for(int j=0;j<donelen;j++){
			if(bs_thread[*n].sgs[i]==donefiles[j]){
				isflag=0;
				break;
			}
		}
		cout<<"isflag:"<<isflag<<endl;
		if(isflag==0) continue;
		string fpath=get_filename(bs_thread[*n].sgs[i]);
        cout<<ip<<" "<<fpath<<endl;
		uint32_t flag=-1;
        uint32_t length=fpath.length();
		number_tmp=write(fd,&flag,sizeof(uint32_t));
		if(number_tmp<sizeof(uint32_t)) return NULL;
		number_tmp=write(fd,&length,sizeof(uint32_t));
		if(number_tmp<sizeof(uint32_t)) return NULL;
		number_tmp=write(fd,fpath.c_str(),fpath.length());
		if(number_tmp<fpath.length()) return NULL;
		if(io_func(fpath,fd)==-1) return NULL;
	}   
	uint32_t over=-1;
	number_tmp=write(fd,&over,sizeof(uint32_t));
	if(number_tmp<sizeof(uint32_t)) return NULL;
	over=0;
	number_tmp=write(fd,&over,sizeof(uint32_t));
    if(number_tmp<sizeof(uint32_t)) return NULL;
	cout<<"wait recv"<<endl;
	m=recv(fd,buff,100,0);
	cout<<"recv ok"<<endl;
	if(m<=0) return NULL;
	buff[m]='\0';
	success[*n]=1;
	cout<<buff<<endl;
    close(fd);	
	
}

int io_func(string fpath,int fd)
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
				return -1;
			}
		}    
		else{
        	tmp=write(fd,base+n*BUF_SIZE,BUF_SIZE);
            if(tmp!=BUF_SIZE){
                cout<<"write error"<<endl;
            	return -1;
            }
		}
        len+=tmp;
    }    
	return 1;
   // long time_next=getTime();
   // cout<<"time:"<<time_next-time_pre<<"ms datasize:"<<len/1024/1024<<"MB throughout:"<<len/1024/1024/(time_next-time_pre)*1000<<"MB/s"<<endl;
   // close(fd);
}
int zms_client(string dir_name)
{
	pthread_t threads[IP_NUM];
    int args[IP_NUM];
	init();
    read_balance();
	dirname=dir_name;	
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
	for(int n=0;n<IP_NUM;n++){
		if(success[n]!=1) return 0;
	}
	return 1;
}


