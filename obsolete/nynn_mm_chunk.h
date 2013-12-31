#ifndef NYNN_MM_CHUNK_BY_SATANSON
#define NYNN_MM_CHUNK_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_config.h>
using namespace nynn::mm::common;
namespace nynn{namespace mm{

template < size_t BLOCK_SIZE=block_size>struct block_t;

template < 
	size_t BLOCK_SIZE=block_size, 
	size_t CHUNK_BLOCK_MAX=chunk_block_max,
	size_t CHUNK_VERTEX_MAX=chunk_vertex_max
> class chunk_t;

struct vertex_t;
struct edge_t; 
struct vertex_t{
	uint64_t source;
	uint64_t nedge;
	uint32_t blkno;
};

struct edge_t{
	time_t timestamp;
	uint64_t sink;
	union{
		void* 	 bval;
		uint64_t uval;
		uint64_t  ival;
		double   fval;
		char 	 cval[8];
	}val;
};
template<size_t BLOCK_SIZE>
struct block_t{
	union{
		struct{
			uint8_t  flag;//0 indicates next block is not  overflow, 1 indicates overflow.
			uint32_t prev;//prev block
			uint32_t next;//next block
			time_t mints;//min timestamp
			time_t maxts;//max timestamp
			uint64_t nedge;
		}hdr;
		uint32_t index[BLOCK_SIZE/sizeof(uint32_t)];
	}blk;
};

template < size_t BLOCK_SIZE, size_t CHUNK_BLOCK_MAX, size_t CHUNK_VERTEX_MAX > 
class chunk_t{
public:
	typedef chunk_t<BLOCK_SIZE,CHUNK_BLOCK_MAX,CHUNK_VERTEX_MAX> chunk_type;
	typedef block_t<BLOCK_SIZE> block;

	static chunk_type* get_chunk(void *shm)
	{
		chunk_type* chk=static_cast<chunk_type*>(shm);
		return chk;
	}

	~chunk_t(){}

	static void init(const string& path)
	{
		unique_ptr<mmap_file_t> chunkmf(new mmap_file_t(path,CHUNK_BLOCK_MAX*BLOCK_SIZE));

		chunk_type*chk=static_cast<chunk_type*>(chunkmf->get_base());

		chk->superblk.nblk=CHUNK_BLOCK_MAX;
		chk->superblk.blksz=BLOCK_SIZE;
		chk->superblk.nvert=0;
		chk->superblk.nused=(sizeof(superblk)+BLOCK_SIZE-1)/BLOCK_SIZE*BLOCK_SIZE;
		chk->superblk.nfree=0;
		chk->superblk.head=0;
		chk->superblk.top=0;

		log_i("nused=%d",chk->superblk.nused);
		log_i("nfree=%d",chk->superblk.nfree);
		for (size_t i=CHUNK_BLOCK_MAX-1;i>=chk->superblk.nused;i--)
			chk->release(i);
	}

    uint32_t getNumberOfEdges(uint64_t vtx)
	{
		if (__builtin_expect(!vertexExist(vtx),0))return 0;
		return getVertex(vtx)->nedge;
	}

	int newVertex(uint64_t vtx,vector<edge_t>&edges)
	{
		if (__builtin_expect(vertexExist(vtx),0))
			pushEdges(vtx,edges);
		if (__builtin_expect(!vertexAvail(vtx),0))
	}
	int delVertex(uint64_t vtx,vector<edge_t>&edges,uint32_t nedge)
	
	template<class Iter>
	int pushEdges(uint64_t vtx,vector<edge_t>&edges)

	int popEdges(uint64_t vtx,vector<edge_t>&edges,uint32_t nedge)

	template<class Iter>
	int readEdges(uint64_t vtx,const Iter &begin,const Iter &end);
	
	template <class Iter>
	int pushABlockOfEdges(uint64_t vtx,const Iter &begin,const Iter &end,block_t*blk)
	{

	}
	template <class Iter>
	int popABlockOfEdges(uint64_t vtx, const Iter &begin,const Iter &end,block_t*blk)
	{

	}

	uint32_t require()
	{
		size_t blkno=0;

		size_t &head=superblk.head;
		size_t &top=superblk.top;
		//the freelist is empty,return 0 indicating failure of allocation.
		if (head==0) {
			blkno=0;
		//the head of freelist is empty.
	    } else if (top==0){
			block* blk=static_cast<block*>(&superblk)+head;
			blkno=head;
			head=blk->blk.index[top];
		//the freelist is not empty,the head of the freelist is not full.
		} else {
			block* blk=static_cast<block*>(&superblk)+head;
			blkno=blk->blk.index[top--];
		}
		superblk.nfree--;
		return blkno;
	}

	void release(uint32_t blkno)
	{
		size_t &head=superblk.head;
		size_t &top=superblk.top;

		//the freelist is empty or head block of freelist is full before releasing.
		//make releasing block be head block of freelist.
		if (head==0 || top+1==BLOCK_SIZE/sizeof(size_t)){
			block *blk=reinterpret_cast<block*>(&superblk)+blkno;
			top=0;
			blk->blk.index[top]=head;
			head=blkno;
		//the freelist is not empty and the head block of the freelist is not full.
		}else {
			block *blk=reinterpret_cast<block*>(&superblk)+head;
			blk->blk.index[++top]=blkno;
		}

		superblk.nfree++;
	}
	
	size_t getNumberOfVertices()const{return superblk.nvert;}
	
	uint64_t infVertex()const
	{
		assert(superblk.nvert>0);
		return superblk.vertices[0].source;
	}

	uint64_t supVertex()const
	{
		assert(superblk.nvert>0);
		return superblk.vertices[superblk.nvert-1].source;
	}

	uint32_t infOffset()const{return 0;}
	uint32_t supOffset()const{return CHUNK_VERTEX_MAX-1;}
	
	bool vertexExist(uint64_t vtx)
	{
		if (__builtin_expect((getNumberOfVertices()==0), 0))return false;
		if (__builtin_expect((infVertex()<=vtx && vtx<=supVertex()),1))return true;
		return false;
	}

	bool vertexAvail(uint64_t vtx)
	{
		if (__builtin_expect((getNumberOfVertices()==0), 0))return false;	
		uint32_t off=vtx-infVertex();
		if (__builtin_expect((infOffset()<=off&&off<=supOffset()),1))return true;
	}

	vertex_t* getVertex(uint64_t vtx)
	{
		if (__builtin_expect(!vertexAvail(vtx),0))return NULL;
		return &superblk.vertices[vtx-infVertex()];
	}
	
private:

	struct{
		uint32_t nblk;  //total number of blocks in chunk.
		uint32_t blksz; //block size in bytes.

		uint32_t nfree; //number of available blocks.
		uint32_t nused; //number of used blocks.
		uint32_t nvert;


		uint32_t head;//the index of first head blocks;
		uint32_t top;

		vertex_t vertices[CHUNK_VERTEX_MAX];
	}superblk;
};


}}
#endif
