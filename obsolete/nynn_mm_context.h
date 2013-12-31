#ifndef NYNN_MM_CONTEXT_BY_SATANSON
#define NYNN_MM_CONTEXT_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_config.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
#include<nynn_mm_meta.h>
using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{

typedef meta_t meta_type;
typedef chunk_t<> chunk_type;
typedef cache_t<> cache_type;

struct once_t{
	int refc;
	int shmid;
	int length;
};

static void exit_on_recv_signal(int signo)
{
	exit(signo);
}

static void add_signal_handler(int signo,void (*handler)(int)){
	struct sigaction sigact;
	sigact.sa_handler=handler;
	sigaction(signo,&sigact,0);
}

class context_t{
public:
	context_t(const string& dir): dir(dir),lockname("nynn_mm.lock"),
		chunkname("chunk/"),oncename("nynn_mm.once")
	{
		add_signal_handler(SIGINT,exit_on_recv_signal);
		add_signal_handler(SIGABRT,exit_on_recv_signal);

		string lockpath=dir+"/"+lockname;
		string oncepath=dir+"/"+oncename;
		string chunkdir=dir+"/"+oncename;
		
		//require file lock.
		log_d("lock file:%s",lockpath.c_str());
		unique_ptr<flock_t> flockptr(new fwlock_t(lockpath));
		raii_flock_t require(flockptr.get());

		//test whether once file exists.
		//if exists,initialize multiprocess-shared non-persistent resources;
		//otherwise,just obtain the handles to resources.
		if (file_exist(oncepath)){
			log_d("obtain resources.");
			mmap_file_t once_mf(oncepath);
			once_t *once=static_cast<once_t*>(once_mf.get_base());
			once->refc++;

			//obtain initialized meta.
			metashm.reset(new shm_t(once->shmid,sizeof(meta_type)));
			meta=static_cast<meta_type*>(metashm->get_base());

			//obtain initialized cache.
			cacheshm.reset(new shm_t(meta->get_cacheshmid()));
			cache=static_cast<cache_type*>(cacheshm->get_base());

		}else{
			log_d("initializing resources.");
			mmap_file_t once_mf(oncepath,sizeof(once_t));
			once_t *once=static_cast<once_t*>(once_mf.get_base());
			once->refc=1;

			shmid_t metashmid(sizeof(meta_type));
			metashm.reset(new shm_t(metashmid));
			once->shmid=metashm->get_shmid();	
			once->length=metashm->get_length();

			//initialize meta.
			meta=meta_type::get_meta(metashm->get_base(),sizeof(meta_type),chunkdir);

			//initialize cache.
			cacheshm.reset(new shm_t(meta->get_cacheshmid(),sizeof(cache_type)));
			cache=cache_type::get_cache(cacheshm->get_base());

		}
		//initialize chunks.
		size_t nchunkentry=meta->get_chunkentrynum();
		for(size_t i=0;i<nchunkentry;i++){
			chunk_entry_t *chk_entry=meta->get_chunkentry(i);
			if (chk_entry->flag & CHUNK_ENTRY_FLAG_LOCAL){
				chunkmf[chk_entry->where].reset(new mmap_file_t(chk_entry->path));
				void* base=chunkmf[chk_entry->where]->get_base();
				chunk[chk_entry->where]=static_cast<chunk_type*>(base);
			}
		}
	}
	~context_t()
	{
		string lockpath=dir+"/"+lockname;
		string oncepath=dir+"/"+oncename;
		
		//require file lock.
		unique_ptr<flock_t> flockptr(new fwlock_t(lockpath));
		raii_flock_t require(flockptr.get());
		mmap_file_t once_mf(oncepath);
		once_t *once=static_cast<once_t*>(once_mf.get_base());
		once->refc--;
		
		if (once->refc==0) {
			//release meta.
			unique_ptr<meta_type> tmpmeta(meta);

			//release cache.
			unique_ptr<cache_type> tmpcache(cache);
			
			//release chunks
			unique_ptr<chunk_type> tmpchunk[META_CHUNK_ENTRY_MAX];
			size_t nchunkentry=meta->get_chunkentrynum();
			for (size_t i=0;i<nchunkentry;i++){
				chunk_entry_t *chk_entry=meta->get_chunkentry(i);
				if (chk_entry->flag & CHUNK_ENTRY_FLAG_LOCAL){
					tmpchunk[chk_entry->where].reset(chunk[chk_entry->where]);
				}
			}
			if(remove(oncepath.c_str())!=0)THROW_ERROR;
		}
		log_d("");
	}


	meta_type* get_meta()const{return meta;}
	cache_type* get_cache()const{return cache;}

	static context_t * get_context()
	{
		if (context.get()==NULL){
			char * data_dir=getenv("NYNN_MM_DATA_DIR");
			if(data_dir==NULL){
				log_w("environment NYNN_MM_DATA_DIR is not set!");
				exit(0);
			}
			context.reset(new context_t(data_dir));
		}
		log_d("Create object of context_t");
		return context.get();
	}

	chunk_type* get_chunk(size_t where)
	{
		if (where<meta->get_chunkentrynum()){
			return chunk[where];
		}else{
			return NULL;
		}
	}
private:

	context_t(const context_t&);
	context_t& operator=(const context_t&);

	string dir;
	string lockname;
	string oncename;
	string chunkname;

	// "dir/nynn_mm.lock" used as flock for controling all shared non-persistent data.
	// "dir/chunk" contain all chunks(local graph data).
	// "dir/nynn_mm.once" used as tag for all shared data initialized only once.
	
	meta_type  *meta;
	cache_type *cache;
	chunk_type *chunk[META_CHUNK_ENTRY_MAX];

	unique_ptr<shm_t> metashm;
	unique_ptr<shm_t> cacheshm;
	unique_ptr<mmap_file_t>     chunkmf[META_CHUNK_ENTRY_MAX];



	//singleton
	static unique_ptr<context_t> context;
};
static struct initialize_context_t{
	initialize_context_t()
	{
		context_t::get_context();
		log_d("Initialized conntext!");
	}
}init;
}}
#endif
