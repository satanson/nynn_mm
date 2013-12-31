#ifndef NYNN_MM_META_BY_SATANSON
#define NYNN_MM_META_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{
//config
//

enum{
	META_CHUNK_PATH_SIZE=meta_chunk_path_size,
	META_CHUNK_ENTRY_MAX=meta_chunk_entry_max,
	META_CHUNK_LOCK_MAX=meta_chunk_lock_max,
	META_CHUNK_LOCK_SLOT_MAX=meta_chunk_lock_slot_max,

	CHUNK_ENTRY_FLAG_LOCAL=1
};

struct chunk_entry_t;
class  meta_t;

struct chunk_entry_t{
	char path[META_CHUNK_PATH_SIZE];
	uint8_t  flag;	
	uint64_t minvtx;
	uint64_t maxvtx;
	uint32_t where;
};

typedef chunk_t<> chunk_type;
typedef cache_t<> cache_type;

class meta_t:public shm_allocator_t{
public:
	static meta_t* get_meta(void* shm,size_t size,const string &chunkdir)
	{
		return new(shm)meta_t(chunkdir);
	}
	~meta_t()
	{	



		//destroy locks
		semid1_t tmp0(metasemid);
		semid1_t tmp1(cachesemid);
		for (size_t i=0;i<META_CHUNK_LOCK_MAX;i++){
			semid1_t tmp2(chunksemid[i]);
		}
	}

	int  get_cacheshmid()const{return cacheshmid;}
	size_t get_chunkentrynum()const{return nchunkentry;}
	chunk_entry_t* get_chunkentry(int i){return &chunkentry[i];}
	int get_metasemid(){return metasemid;}
	int get_cachesemid(){return cachesemid;}
	int get_chunksemid(int i){return chunksemid[i];}
private:
	meta_t(const string& chunkdir)
	{

		//assert(chunkdir.size()<sizeof(chunkdir));
		if (!(chunkdir.size()<sizeof(chunkdir))){
			log_a("assert(strlen(chunkdir)<sizeof(chunkdir):failure");
			THROW_ERROR_WITH_ERRNO(EINVAL);
		}
		memset(chunkdir,0,sizeof(chunkdir));
		strcpy(chunkdir,chunkdir.c_str());

		//initialize cacheshmid
		shmid_t cacheshmid(sizeof(cache_type));
		cacheshmid=cacheshmid.get_shmid();

		//initialize chunk entries.
		//get local chunk entries.
		memset(chunkentry,0,sizeof(chunkentry));

		glob_t g;
		g.gl_offs=0;
		string pat=chunkdir+"/chunk*.dat";
		glob(pat.c_str(),0,NULL,&g);
		//assert(g.gl_pathc<=META_CHUNK_ENTRY_MAX);
		if (!(g.gl_pathc<=META_CHUNK_ENTRY_MAX)){
			log_a("assert(g.gl_pathc<=META_CHUNK_ENTRY_MAX):failue");
			THROW_ERROR_WITH_ERRNO(EINVAL);
		}

		nchunkentry=g.gl_pathc;
		for (size_t i=0;i<nchunkentry;i++){
			//assert(strlen(g.gl_pathv[i])<META_CHUNK_PATH_SIZE);
			if (!(strlen(g.gl_pathv[i])<META_CHUNK_PATH_SIZE)){
				log_a("assert(strlen(g.gl_pathv[i])<META_CHUNK_PATH_SIZE):failue");
				THROW_ERROR_WITH_ERRNO(EINVAL);
			}
			strcpy(chunkentry[i].path,g.gl_pathv[i]);
			chunkentry[i].flag|=CHUNK_ENTRY_FLAG_LOCAL;
			chunkentry[i].where=i;
			unique_ptr<mmap_file_t> chunkmf(new mmap_file_t(chunkentry[i].path));
			chunk_type* chk=static_cast<chunk_type*>(chunkmf->get_base());
			chunkentry[i].minvtx=chk->get_minvertex();
			chunkentry[i].maxvtx=chk->get_maxvertex();
		}

		globfree(&g);

		
		//initialize locks.	
		metasemid=semid0_t(1).get_semid();
		cachesemid=semid0_t(META_CHUNK_LOCK_SLOT_MAX).get_semid();
		
		for (size_t i=0;i<META_CHUNK_LOCK_MAX;i++)
			chunksemid[i]=semid0_t(META_CHUNK_LOCK_SLOT_MAX).get_semid();
	}

	meta_t(const meta_t&);
	meta_t& operator=(const meta_t&);

	char chunkdir[META_CHUNK_PATH_SIZE];
	int cacheshmid;
	size_t nchunkentry;
	chunk_entry_t chunkentry[META_CHUNK_ENTRY_MAX];
	
	int metasemid;//1*1 locks for meta.
	int cachesemid;//1*META_CHUNK_LOCK_SLOT_MAX locks for cache.
	int chunksemid[META_CHUNK_LOCK_MAX];//32*META_CHUNK_LOCK_SLOT_MAX locks for chunk
};
}}
#endif
