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
#include<set>
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
#include<climits>
#include<cstddef>

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
	VSNPRINTF_BUFF_SIZE=512,


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



class MmapFile{
public:
	//create a shared mapping file for already existed file
	explicit MmapFile(const string& m_path) throw(NynnException)
		:m_path(m_path),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throwNynnException(errno,NULL);

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throwNynnException(errno,NULL);

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throwNynnException(errno,NULL);

		if (close(m_fd)!=0)		
			throwNynnException(errno,NULL);
	}

	//create a shared mapping file for already existed file
	MmapFile(const string& m_path,size_t m_length,off_t m_offset)throw(NynnException)
		:m_path(m_path),m_offset(m_offset),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throwNynnException(errno,NULL);

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throwNynnException(errno,NULL);

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);
		if (m_base==MAP_FAILED)
			throwNynnException(errno,NULL);

		if (close(m_fd)!=0)		
			throwNynnException(errno,NULL);

	}

	// create a private mapping file for a new file
	MmapFile(const string& m_path,size_t m_length)throw(NynnException)
		:m_path(m_path),m_length(m_length),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
		if (m_fd<0)
			throwNynnException(errno,NULL);

		if (lseek(m_fd,m_length-4,SEEK_SET)==-1)
			throwNynnException(errno,NULL);

		if (write(m_fd,"\0\0\0\0",4)!=4)
			throwNynnException(errno,NULL);

		m_base=mmap(NULL,m_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throwNynnException(errno,NULL);

		if (close(m_fd)!=0)		
			throwNynnException(errno,NULL);
	}

	void lock(void* addr,size_t m_length)throw(NynnException)
	{

		if (!checkPagedAlignment(addr,m_length))
			throwNynnException(ENOMEM,NULL);

		if (mlock(addr,m_length)!=0)
			throwNynnException(errno,NULL);

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
			throwNynnException(ENOMEM,NULL);

		if (munlock(addr,m_length)!=0)
			throwNynnException(errno,NULL);

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
			throwNynnException(ENOMEM,NULL);

		if (msync(addr,m_length,flags)!=0)
			throwNynnException(errno,NULL);

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
		if (semid==-1) throwNynnException(errno,NULL);
		
		uint16_t *array=new uint16_t[slot_max];
		std::fill(array,array+slot_max,0);
		if (semctl(semid,0,SETALL,array)==-1)throwNynnException(errno,NULL);
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
		if (semctl(semid,0,IPC_RMID)==-1)throwNynnException(errno,NULL);
	}

	int    semid;
};	

class Lockop {

public:

	Lockop(int semid,int slot )throw(NynnException):semid(semid),slot(slot)
	{

		struct seminfo si;
		if (semctl(0,0,IPC_INFO,&si)==-1)throwNynnException(errno,NULL);
		int semmsl=si.semmsl;
		int semopm=si.semopm;
		log_i("semmsl=%d",semmsl);
		log_i("semopm=%d",semopm);
		

		struct semid_ds ds;
		if (semctl(semid,0,IPC_STAT,&ds)!=0)throwNynnException(errno,NULL);

		if (slot>=ds.sem_nsems)throwNynnException(EINVAL,NULL);

		struct sembuf sop;
		sop.sem_op=-1;
		sop.sem_num=slot;
		sop.sem_flg=SEM_UNDO;
		
		if (semop(semid,&sop,1)!=0)throwNynnException(errno,NULL);
	}

	~Lockop()throw(NynnException)
	{

		struct sembuf sop;
		sop.sem_op=1;
		sop.sem_num=slot;
		
		if (semop(semid,&sop,1)!=0)throwNynnException(errno,NULL);
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
			throwNynnException(errno,NULL);
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
		if(m_base==(void*)-1)throwNynnException(errno,NULL);
	}
	
	explicit Shm(const Shmid& id)throw(NynnException):
		m_shmid(id.m_shmid),m_length(id.m_length)
	{
		m_base=shmat(m_shmid,NULL,0);
		if(m_base==(void*)-1)throwNynnException(errno,NULL);
	}

	~Shm()
	{
		if(shmdt(m_base)==-1)
			throwNynnException(errno,NULL);

		struct shmid_ds ds;
		if (shmctl(m_shmid,IPC_STAT,&ds)==-1)
			throwNynnException(errno,NULL);

		if (ds.shm_nattch>0)return;
		if (shmctl(m_shmid,IPC_RMID,NULL)==-1)
			throwNynnException(errno,NULL);
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
			throwNynnException(ENOENT,NULL);

		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd==-1)
			throwNynnException(ENOENT,NULL);
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
			throwNynnException(errno,NULL);
	}
	
	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(errno,NULL);

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
		throwNynnException(errno,NULL);
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
			throwNynnException(errno,NULL);
	}

	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throwNynnException(errno,NULL);
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
		throwNynnException(errno,NULL);
	}
};

class FlockRAII{
public:
	explicit FlockRAII(Flock* lck):lock(lck){lock->lock(0,0);}
			~FlockRAII(){lock->unlock(0,0);}
private:
	Flock *lock;
};
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
	pthread_spinlock_t m_mutex;
public:
	Monitor(){pthread_spin_init(&this->m_mutex,PTHREAD_PROCESS_PRIVATE);}
	~Monitor(){pthread_spin_destroy(&this->m_mutex);}
	pthread_spinlock_t* get(){return &this->m_mutex;}
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
	Synchronization(Monitor* m):m_monitor(m){pthread_spin_lock(m_monitor->get());}
	~Synchronization(){pthread_spin_unlock(m_monitor->get());}
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
}}}
#endif
