#include<nynn_common.hpp>
int main()
{
	uint32_t addr;
	string hostname;
	char buf[INET_ADDRSTRLEN];
	if (-1==gethostname(buf,INET_ADDRSTRLEN)){
		log_e(errno);
		return 0;
	}
	hostname.resize(0);
	hostname+=buf;

	struct addrinfo  hint, *res, *p;
	memset(&hint,0,sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_flags = AI_ADDRCONFIG;
	
	if (-1==getaddrinfo(hostname.c_str(),NULL,&hint,&res)){
		log_e(errno);
		return 0;
	}
	if (res!=NULL){
		p=res;
		do{
			addr=((sockaddr_in*)p->ai_addr)->sin_addr.s_addr;

			if (addr!=0&&(addr&0xff)!=0x7f){
				cout<<ip2string(addr)<<endl;
			}
			p=p->ai_next;
		}while(p!=NULL);
		freeaddrinfo(res);

	}else{
		return 0;
	}
}
