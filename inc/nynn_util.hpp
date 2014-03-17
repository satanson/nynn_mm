#ifndef NYNN_UTIL_HPP_BY_SATANSON
#define NYNN_UTIL_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_log.hpp>
namespace nynn{

uint32_t parse_int(const char* s,uint32_t value);

template<typename T>
string to_string(T const& a){
	return ((stringstream&)(stringstream()<<a)).str();
}

void set_signal_handler(int signum,void(*handler)(int));

void nanosleep_for(uint32_t ns);
void rand_nanosleep();

int rand_int();

bool file_exist(const string& path);

string get_host();
uint32_t get_ip(string &hostname);
string ip2string(uint32_t ip);
uint32_t string2ip(string const& s);

time_t string2time(string const& s);
string time2string(time_t t);

char* ltrim(const char *chars,const char *src, char *dest);
char* rtrim(const char *chars,const char *src, char *dest);
char* chop(const char ch,const char *src,char *dest);

vector<string> get_array1d_of_words(istream & inputstream); 
vector<vector<string> > get_array2d_of_words(istream & inputstream); 

inline void add_signal_handler(int signum,void(*handler)(int)){
	struct sigaction sigact;
	sigact.sa_handler=handler;
	sigaction(signum,&sigact,NULL);
}
string time2string(time_t t)
{
	struct tm tt;
	char buff[32];
	strftime(buff,sizeof(buff),"%Y-%m-%d %T",gmtime_r(&t,&tt));
	return string(buff);
}

time_t string2time(string const& str)
{
	struct tm tt;
	time_t t;
	strptime(str.c_str(),"%Y-%m-%d %T",&tt);
	return mktime(&tt);
}

inline uint32_t parse_int(const char* s,uint32_t value){
	if (s==NULL){
		log_w("s shouldn't be NULL");
		return value;
	}
	uint32_t value2=strtoul(s,NULL,0);
	if (value2==0||value2==ULONG_MAX){
		log_w("fail to convert %s to uint32_t",s);
		return value;
	}
	return value2;
}

inline void nanosleep_for(uint32_t ns){
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=ns;
	nanosleep(&ts,NULL);
}

inline void rand_nanosleep(){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	ts.tv_sec=0;
	ts.tv_nsec=rand_r((uint32_t*)&ts.tv_nsec);
	cout<<"sleep "<<ts.tv_nsec<<"ns"<<endl;
	nanosleep(&ts,NULL);
}


inline string get_host()
{
	char host[128];
	gethostname(host,sizeof(host));
	return string(host,host+strlen(host));
}

inline uint32_t get_ip(){
	uint32_t addr;
	char host[INET_ADDRSTRLEN];
	if (-1==gethostname(host,INET_ADDRSTRLEN)){
		log_e(errno);
		return 0;
	}

	struct addrinfo  hint, *res, *p;
	memset(&hint,0,sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_flags = AI_ADDRCONFIG;
	
	if (-1==getaddrinfo(host,NULL,&hint,&res)){
		log_e(errno);
		return 0;
	}

	if (res!=NULL){
		p=res;
		do{
			addr=((sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
			//inet_ntop(AF_INET,&addr,buf,INET_ADDRSTRLEN);
			//if (strcmp(buf,"127.0.0.1")!=0){
			if ( addr!=0 && (addr&0xff)!=0x7f)
				return addr;

			p=p->ai_next;
		}while(NULL!=p);
		freeaddrinfo(res);
	}else{
		return 0;
	}
}
inline string ip2string(uint32_t ip){
	char buff[16];
	return string(inet_ntop(AF_INET,&ip,buff,16));
}
inline uint32_t string2ip(string const& s){
	int ip;
	inet_pton(AF_INET,s.c_str(),&ip);
	return ip;
}
inline char* ltrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if (s.find_first_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{
		string s1=s.substr(s.find_first_not_of(chars));
		strcpy(dest,s1.c_str());
	}
	return dest;
}

inline char* rtrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if(s.find_last_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{		
		string s1=s.substr(0,s.find_last_not_of(chars)+1);
		strcpy(dest,s1.c_str());
	}
	return dest;
}

inline char* chop(const char ch,const char *src,char *dest)
{	
	if (strlen(src)==0){
		dest[0]='\0';
	}else{
		string s(src);
		string s1=s.substr(0,s.find_first_of(ch));
		strcpy(dest,s1.c_str());
	}
	return dest;
}

inline vector<string> get_array1d_of_words(istream &inputstream)
{
	vector<string> t;
	string word;
	t.resize(0);
	while(inputstream>>word){
		t.push_back(word);
	}
	return t;
}

inline vector<vector<string> > get_array2d_of_words(istream &inputstream)
{
	vector<vector<string> > array;
	vector<string> t;
	stringstream ss;
	char buff[64];

	while (!inputstream.eof()){
		do{
			inputstream.setstate(inputstream.rdstate()&~ios::failbit);
			inputstream.getline(buff,64,'\n');
			ss<<buff;
		}while(inputstream.fail()&&!inputstream.eof());

		if(ss.str().size()!=0)array.push_back(get_array1d_of_words(ss));
		ss.str(string(""));
		ss.clear();
	}
	return array;
}	

inline bool file_exist(const string& path)
{
	struct stat st;
	if (stat(path.c_str(),&st)==0)return true;
	return false;
}

}
#endif
