#ifndef NYNN_MM_COMMON_BY_SATANSON
#define NYNN_MM_COMMON_BY_SATANSON

#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<memory>
#include<string>
#include<vector>
#include<list>
#include<map>
#include<exception>
#include<algorithm>

#include<cstring>
#include<cstdlib>
#include<cerrno>
#include<cstdarg>
#include<cassert>
#include<cstdio>
#include<ctime>
#include<csignal>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/mman.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/socket.h>


#include<unistd.h>
#include<fcntl.h>
#include<execinfo.h>
#include<glob.h>
#include<netdb.h>

#include<arpa/inet.h>

using namespace std;

typedef unsigned long long int uint64_t;
typedef long long int int64_t;
typedef unsigned short int uint16_t;
typedef short int int16_t;
typedef unsigned char uint8_t;

namespace nynn{ namespace mm{ namespace common{
//declaration
enum{
	ERR_BUFF_SIZE=128,
	ERR_MSG_RESERVED_SIZE=256,
	BACKTRACE_FRAME_SIZE=20,
	VSNPRINTF_BUFF_SIZE=512,

	//log level
	LOG_INFO=0,
	LOG_WARN=1,
	LOG_ERROR=2,
	LOG_ASSERT=3,
	LOG_DEBUG=4,

	SEMGET_TRY_MAX=1000,
	//rwlock type
	RWLOCK_SHARED=1,
	RWLOCK_EXCLUSIVE=0,

	DUMMY
};


class NynnException;
class MmapFile;

struct ShmAllocator;
struct Inetaddr;
class Lockop;
class Shm;

class Flock;
class Frlock;
class Fwlock;

class FlockRAII;

class Monitor;
class Synchronized;

string strerr(int errnum);
void vlog(ostream &out,const char *file,const int line,
		const char *function,const int level,int errnum,const char *fmt,va_list ap);
void log_debug(ostream&out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...);
void log(ostream& out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...);
int rand_int();
bool file_exist(const string& m_path);
uint32_t gethostaddr(string &hostname);
time_t str2time(const char*str);
string time2str(time_t t);

char* ltrim(const char *chars,const char *src, char *dest);
char* rtrim(const char *chars,const char *src, char *dest);
char* chop(const char ch,const char *src,char *dest);

vector<string>& gettuple(istream & inputstream,vector<string>& t); 
vector<vector<string> >& gettuplearray(istream & inputstream,vector<vector<string> >& array); 

#define log_i(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_INFO,0,(msg),##__VA_ARGS__)
#define log_w(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_WARN,0,(msg),##__VA_ARGS__)
#define log_e(errnum) \
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_ERROR,errnum,NULL)
#define log_a(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_ASSERT,0,(msg),##__VA_ARGS__)
#define log_d(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_DEBUG,0,(msg),##__VA_ARGS__)

#define throwNynnException(msg) \
   	throw NynnException(__FILE__,__LINE__,__FUNCTION__,(msg))

inline uint32_t gethostaddr(string &hostname)
{
	uint32_t addr;
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
			//inet_ntop(AF_INET,&addr,buf,INET_ADDRSTRLEN);
			//if (strcmp(buf,"127.0.0.1")!=0){
			if (ntohl(addr)&0xff000000u!=0xff000000u)return addr;
			p=p->ai_next;
		}while(NULL!=p);
		freeaddrinfo(res);

	}else{
		return 0;
	}

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
inline vector<string> & gettuple(istream &inputstream,vector<string>&t)
{
	string word;
	t.resize(0);
	word.reserve(64);
	while(!inputstream.eof()){
		inputstream>>word;
		t.push_back(word);
	}
	return t;
}

inline vector<vector<string> >& gettuplearray(istream &inputstream,vector<vector<string> >& array)
{
	vector<string> t;
	stringstream ss;
	char buff[64];

	while (!inputstream.eof()){
		do{
			inputstream.setstate(inputstream.rdstate()&~ios::failbit);
			inputstream.getline(buff,64,'\n');
			ss<<buff;
		}while(inputstream.fail()&&!inputstream.eof());

		if(ss.str().size()!=0)array.push_back(gettuple(ss,t));
		ss.str(string(""));
		ss.clear();
	}
	return array;
}	

inline bool file_exist(const string& m_path)
{
	struct stat st;
	if (stat(m_path.c_str(),&st)==0)return true;
	if (errno!=ENOENT)log_e(errno);
	return false;
}


class NynnException:public exception{
public:
	NynnException(
			const char* file,
			const int line,
			const char*function,
			const char *msg)
	{
		stringstream pack;
		cout<<"NynnException"
			<<"@"<<file
			<<"#"<<line
			<<"-"<<function
			<<":"<<msg
			<<endl;

		m_msg+=msg;
		m_framesize=backtrace(m_frames,BACKTRACE_FRAME_SIZE);
	}
	~NynnException()throw(){}

	const char* what()const throw()
	{
		return m_msg.c_str();
	}

	void printBacktrace(){
		char **symbols=backtrace_symbols(m_frames,m_framesize);
		cerr<<"backtrace:"<<endl;
		if (symbols!=NULL) {
			for (size_t i=0;i<m_framesize;i++)
				cerr<<symbols[i]<<endl;
		}
		free(symbols);
	}

private:
	string m_msg;
	void*  m_frames[BACKTRACE_FRAME_SIZE];
	size_t m_framesize;
};

class MmapFile{
public:
	//create a shared mapping file for already existed file
	explicit MmapFile(const string& m_path) throw(NynnException)
		:m_path(m_path),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throwNynnException(strerr(errno).c_str());

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throwNynnException(strerr(errno).c_str());

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throwNynnException(strerr(errno).c_str());

		if (close(m_fd)!=0)		
			throwNynnException(strerr(errno).c_str());
	}

	//create a shared mapping file for already existed file
	MmapFile(const string& m_path,size_t m_length,off_t m_offset)throw(NynnException)
		:m_path(m_path),m_offset(m_offset),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throwNynnException(strerr(errno).c_str());

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throwNynnException(strerr(errno).c_str());

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);
		if (m_base==MAP_FAILED)
			throwNynnException(strerr(errno).c_str());

		if (close(m_fd)!=0)		
			throwNynnException(strerr(errno).c_str());

	}

	// create a private mapping file for a new file
	MmapFile(const string& m_path,size_t m_length)throw(NynnException)
		:m_path(m_path),m_length(m_length),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR|O_CREAT|O_EXCL);
		if (m_fd<0)
			throwNynnException(strerr(errno).c_str());

		if (lseek(m_fd,m_length-4,SEEK_SET)==-1)
			throwNynnException(strerr(errno).c_str());

		if (write(m_fd,"\0\0\0\0",4)!=4)
			throwNynnException(strerr(errno).c_str());

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throwNynnException(strerr(errno).c_str());

		if (close(m_fd)!=0)		
			throwNynnException(strerr(errno).c_str());
	}

	void lock(void* addr,size_t m_length)throw(NynnException)
	{

		if (!checkPagedAlignment(addr,m_length))
			throwNynnException(strerr(ENOMEM).c_str());

		if (mlock(addr,m_length)!=0)
			throwNynnException(strerr(errno).c_str());

		return;
	}

	void lock()throw(NynnException)
	{
		try{
			lock(m_base,m_length);
		}catch (NynnException& err) {
			throw err;
		}

		return; 
	}

	void unlock(void* addr,size_t m_length)throw(NynnException)
	{

		if (!checkPagedAlignment(addr,m_length))
			throwNynnException(strerr(ENOMEM).c_str());

		if (munlock(addr,m_length)!=0)
			throwNynnException(strerr(errno).c_str());

		return;
	}

	void unlock()throw(NynnException)
	{
		try{
			unlock(m_base,m_length);
		}catch (NynnException& err) {
			throw err;
		}

		return; 
	}

	void sync(void* addr,size_t m_length,int flags)throw(NynnException)
	{

		if (!checkPagedAlignment(addr,m_length))
			throwNynnException(strerr(ENOMEM).c_str());

		if (msync(addr,m_length,flags)!=0)
			throwNynnException(strerr(errno).c_str());

		return;
	}

	void sync(int flags)throw(NynnException)
	{
		try{
			sync(m_base,m_length,flags);
		}catch (NynnException& err){
			throw err;
		}
		return;
	}


	~MmapFile()
	{
		try{
			sync(MS_SYNC|MS_INVALIDATE);
		}catch (NynnException& err) {
			cerr<<err.what()<<endl;
		}
		//munmap(m_base,m_length);
	}

	void *getBaseAddress()const
	{
		return m_base;
	}

	size_t getLength()const
	{
		return m_length;
	}


private:
		//forbid copying object 
	MmapFile(const MmapFile&);
	MmapFile& operator=(const MmapFile&);

	bool checkPagedAlignment(void* &addr,size_t &m_length)
	{
		size_t pagesize=static_cast<size_t>(sysconf(_SC_PAGESIZE));
		// round down addr by page boundary
		char* taddr=static_cast<char*>(addr);
		char* tbase=static_cast<char*>(m_base);

		m_length += ((size_t)taddr)%pagesize;
		taddr  -= ((size_t)taddr)%pagesize;
		addr=static_cast<void*>(taddr);

		// check [addr,addr+m_length] in [m_base,m_base+m_length]
		if (taddr-tbase>=0 && taddr-tbase<=m_length-m_length)
			return true;
		else 
			return false;
	}

	string 	m_path;
	int 	m_fd;
	size_t 	m_length;
	off_t 	m_offset;
	void*   m_base;
};

struct ShmAllocator{
	void* operator new(size_t size,void*buff){return buff;}
	void  operator delete(void*buff,size_t size){ }
};
struct Semid0{

	Semid0(size_t slots)throw(NynnException):slot_max(slots)
	{	
		semid=semget(IPC_PRIVATE,slot_max,IPC_CREAT|IPC_EXCL|0700);
		if (semid==-1) throwNynnException(strerr(errno).c_str());
		
		uint16_t *array=new uint16_t[slot_max];
		std::fill(array,array+slot_max,0);
		if (semctl(semid,0,SETALL,array)==-1)throwNynnException(strerr(errno).c_str());
	}

	~Semid0(){}

	size_t get_slot_max()const{return slot_max;}
	int    get_semid()const {return semid;}
	
	int    semid;		
	size_t slot_max;
};

struct Semid1{
	Semid1(int semid):semid(semid){}
	
	~Semid1()
	{
		if (semctl(semid,0,IPC_RMID)==-1)throwNynnException(strerr(errno).c_str());
	}

	int    semid;
};	

class Lockop {

public:

	Lockop(int semid,int slot )throw(NynnException):semid(semid),slot(slot)
	{

		struct seminfo si;
		if (semctl(0,0,IPC_INFO,&si)==-1)throwNynnException(strerr(errno).c_str());
		int semmsl=si.semmsl;
		int semopm=si.semopm;
		log_i("semmsl=%d",semmsl);
		log_i("semopm=%d",semopm);
		

		struct semid_ds ds;
		if (semctl(semid,0,IPC_STAT,&ds)!=0)throwNynnException(strerr(errno).c_str());

		if (slot>=ds.sem_nsems)throwNynnException(strerr(EINVAL).c_str());

		struct sembuf sop;
		sop.sem_op=-1;
		sop.sem_num=slot;
		sop.sem_flg=SEM_UNDO;
		
		if (semop(semid,&sop,1)!=0)throwNynnException(strerr(errno).c_str());
	}

	~Lockop()throw(NynnException)
	{

		struct sembuf sop;
		sop.sem_op=1;
		sop.sem_num=slot;
		
		if (semop(semid,&sop,1)!=0)throwNynnException(strerr(errno).c_str());
	}

private:

	//disallow copy.
	Lockop(const Lockop&);
	Lockop& operator=(const Lockop&);
	//disallow created on free store(heap);
	void*operator new(size_t);
	void*operator new(size_t,void*);
	int semid;
	int slot;
};

struct Shmid{
	int m_shmid;
	size_t m_length;
	explicit Shmid(size_t m_length)throw(NynnException):
		m_shmid(0),m_length(m_length)
	{
		m_shmid=shmget(IPC_PRIVATE,m_length,IPC_CREAT|IPC_EXCL|0700);
		if (m_shmid==-1)
			throwNynnException(strerr(errno).c_str());
	}
	~Shmid(){}
	int get_shmid()const{return m_shmid;}
	size_t getLength()const{return m_length;}

private:
	Shmid(const Shmid&);
	Shmid& operator=(const Shmid&);
};

class Shm{
public:

	explicit Shm(int m_shmid,size_t m_length=0)throw(NynnException):
		m_shmid(m_shmid),m_length(m_length),m_base(0)
	{ 
		m_base=shmat(m_shmid,NULL,0);
		if(m_base==(void*)-1)throwNynnException(strerr(errno).c_str());
	}
	
	explicit Shm(const Shmid& id)throw(NynnException):
		m_shmid(id.m_shmid),m_length(id.m_length)
	{
		m_base=shmat(m_shmid,NULL,0);
		if(m_base==(void*)-1)throwNynnException(strerr(errno).c_str());
	}

	~Shm()
	{
		if(shmdt(m_base)==-1)
			throwNynnException(strerr(errno).c_str());

		struct shmid_ds ds;
		if (shmctl(m_shmid,IPC_STAT,&ds)==-1)
			throwNynnException(strerr(errno).c_str());

		if (ds.shm_nattch>0)return;
		if (shmctl(m_shmid,IPC_RMID,NULL)==-1)
			throwNynnException(strerr(errno).c_str());
	}

	void* getBaseAddress()const{return m_base;}
	size_t getLength()const{return m_length;}
	int   get_shmid()const{return m_shmid;}
private:
	Shm(const Shm&);
	Shm& operator=(const Shm&);

	int    m_shmid;
	size_t m_length;
	void*  m_base;
};

class Flock{
protected:
	string m_path;
	int m_fd;

	Flock(const string& m_path)throw(NynnException):m_path(m_path){
		if (!file_exist(m_path.c_str()))
			throwNynnException(strerr(ENOENT).c_str());

		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd==-1)
			throwNynnException(strerr(ENOENT).c_str());
	}

public:
	virtual void lock(off_t start,off_t m_length)=0;
	virtual void unlock(off_t start,off_t m_length)=0;
	virtual bool trylock(off_t start,off_t m_length)=0;
	virtual ~Flock() { close(m_fd);}
};

class Frlock:public Flock{
public:
	Frlock(const string& m_path):Flock(m_path){}
	~Frlock(){}
	virtual void lock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(strerr(errno).c_str());
	}
	
	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(strerr(errno).c_str());

	}
	virtual bool trylock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		throwNynnException(strerr(errno).c_str());
	}
};

class Fwlock:public Flock{
public:	
	Fwlock(const string&path):Flock(path){}
	~Fwlock(){} 
	virtual void lock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(strerr(errno).c_str());
	}

	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(strerr(errno).c_str());
	}

	virtual bool trylock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		throwNynnException(strerr(errno).c_str());
	}
};

class FlockRAII{
public:
	explicit FlockRAII(Flock* lck):lock(lck){lock->lock(0,0);}
			~FlockRAII(){lock->unlock(0,0);}
private:
	Flock *lock;
};

inline string strerr(int errnum)
{
	char buff[ERR_BUFF_SIZE];
	string s;
	s.reserve(ERR_BUFF_SIZE);
	memset(buff,0,ERR_BUFF_SIZE);
#if (POSIX_C_SOURCE >= 200112L || XOPEN_SOURCE >= 600) && ! GNU_SOURCE
	//XSI-compliant
	if (strerror_r(errnum,buff,ERR_BUFF_SIZE)==0)
		s=buff;
	else
		s="Unknown Error!";
#else
	//GNU-specific
	char *msg=strerror_r(errnum,buff,ERR_BUFF_SIZE);
	if (msg!=NULL)
		s=msg;
	else
		s="Unknown Error!";
#endif
	return s;
}

inline void log_debug(ostream&out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...)
{
#ifdef DEBUG
	va_list ap;
	va_start(ap,fmt);
	vlog(out,file,line,function,level,errnum,fmt,ap);
	va_end(ap);
#endif
}

inline void log(ostream& out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vlog(out,file,line,function,level,errnum,fmt,ap);
	va_end(ap);

}

inline void vlog(ostream &out,const char *file,const int line,
		const char *function,const int level,int errnum,const char *fmt,va_list ap)
{
	const char* logs[5]={"INFO","WARN","ERROR","ASSERT","DEBUG"};
	stringstream pack;
	string s;
	char buff[VSNPRINTF_BUFF_SIZE];
	pack<<"####"<<logs[level]<<"@"
		<<file
		<<"#"<<line
		<<"-"<<function<<"():";
	if (errnum!=0){
		pack<<"["<<errnum<<"]"
			<<strerr(errnum);
	}else{
		vsnprintf(buff,VSNPRINTF_BUFF_SIZE,fmt,ap);
		buff[VSNPRINTF_BUFF_SIZE-1]='\0';
		pack<<buff;
	}

	out<<pack.str()<<endl;
}

inline int rand_int()
{
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=1;
	clock_nanosleep(CLOCK_MONOTONIC,0,&ts,NULL);

	clock_gettime(CLOCK_MONOTONIC,&ts);
	unsigned seed=static_cast<unsigned>(0x0ffffffffl & ts.tv_nsec);
	return rand_r(&seed);
}

class Monitor{
	pthread_mutex_t m_mutex;
public:
	Monitor(){pthread_mutex_init(&this->m_mutex,NULL);}
	~Monitor(){pthread_mutex_destroy(&this->m_mutex);}
	pthread_mutex_t* get(){return &this->m_mutex;}
};
class RWLock{
	pthread_rwlock_t m_rwlock;
public:
	RWLock() { pthread_rwlock_init(&this->m_rwlock,NULL);}
	~RWLock() { pthread_rwlock_destroy(&this->m_rwlock);}
	pthread_rwlock_t* get() {return &this->m_rwlock; }
	
};

class Synchronization{
	Monitor *m_monitor;
public:
	Synchronization(Monitor* m):m_monitor(m){pthread_mutex_lock(m_monitor->get());}
	~Synchronization(){pthread_mutex_unlock(m_monitor->get());}
};	
class SharedSynchronization{
	RWLock *m_lock;
public:
	SharedSynchronization(RWLock *lock):m_lock(lock)
	{ 
		pthread_rwlock_rdlock(m_lock->get());
	}
	~SharedSynchronization(){pthread_rwlock_unlock(m_lock->get());}
};

class ExclusiveSynchronization{
	RWLock *m_lock;
public:
	ExclusiveSynchronization(RWLock *lock):m_lock(lock)
	{ 
		pthread_rwlock_wrlock(m_lock->get());
	}
	~ExclusiveSynchronization(){pthread_rwlock_unlock(m_lock->get());}
};
string time2str(time_t t)
{
	struct tm tt;
	char buff[32];
	strftime(buff,sizeof(buff),"%Y-%m-%d %T",gmtime_r(&t,&tt));
	return string(buff);
}

time_t str2time(const char* str)
{
	struct tm tt;
	time_t t;
	strptime(str,"%Y-%m-%d %T",&tt);
	return mktime(&tt);
}

}}}
#endif
