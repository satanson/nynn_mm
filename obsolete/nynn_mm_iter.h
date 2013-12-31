#ifndef NYNN_MM_ITER_BY_SATANSON
#define NYNN_MM_ITER_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
#include<nynn_mm_meta.h>
#include<nynn_mm_context.h>

using namespace nynn::mm::common;
using namespace nynn::mm;
namespace nynn{namespace mm{
template<typename T>
class iter_t;
class vertex_iter_t;
class vertex_local_iter_t;
class vertex_all_iter_t;
class edge_iter_t;

template<typename T>
class iter_t{
protected:
	T next;
public:
	iter_t();
	virtual bool has_next()=0;
	virtual T* next()=0;
	virtual ~iter_t(){}
};

class vertex_iter_t:public iter_t<vertex_t>{
public:
	vertex_iter_t();
	virtual bool has_next()=0;
	virtual vertex_t* next()=0;
	virtual ~vertex_iter_t(){}

};

class vertex_local_iter_t:public vertex_iter_t{
	enum{VERTEX_BUF_CAPACITY=128};
public:
	explicit vertex_local_iter_t():vtxbuf(VERTEX_BUF_CAPACITY),
		chk_cursor(0),vtx_cursor(0),chk_num(0),vtx_num(0)
	{
		ctx=context_t::get_context();
		meta=ctx->get_meta();

		fetch_local_data();
	}

	bool has_next()
	{
		if (chk_cursor==chk_num && vtx_cursor==vtx_num && vtxbuf_iter==vtxbuf.end())
			return false;
		return true;
	}
	vertex_t* next()
	{
		if (has_next()){
			if (vtxbuf_iter==vtxbuf.end())fetch_local_data();
			return &*vtxbuf_iter++;
		}else {
			return NULL;
		}
	}

	~vertex_local_iter_t();
private:
	void fetch_local_data(){
		//lock meta 
		lockop_t get(meta->get_metasemid(),0);
	
		//fetch rest vertices from old chunk to vertex buffer.
		if (vtx_cursor!=vtx_num)
		{
			//copy vertices from chunk to vertice buffer.
			//set vertice iterator.
			vtxbuf.resize(std::min<size_t>(vtx_num-vtx_cursor,VERTEX_BUF_CAPACITY));
			vertex_t *vtx_begin=chunk->get_vertex(vtx_cursor);
			vtxbuf_iter=vtxbuf.begin();
			std::copy(vtx_begin,vtx_begin+vtxbuf.size(),vtxbuf_iter);
			//move vertex cursor forward.
			vtx_cursor+=vtxbuf.size();
			return ;
		}

		//fetch vertices from new chunk to vertex buffer.
		chk_num = meta->get_chunkentrynum();
		while(chk_cursor<chk_num){
			chunk_entry_t* chk_entry=meta->get_chunkentry(chk_cursor);
			//check chunk if stored in local.
			if (chk_entry->flag & CHUNK_ENTRY_FLAG_LOCAL){
				//get first local chunk.
				chunk=ctx->get_chunk(chk_entry->where);
				vtx_cursor=0;
				//get number of vertices in the local chunk.
				vtx_num=chunk->get_vertexnum();
				//copy vertices from chunk to vertice buffer.
				//set vertice iterator.
				vtxbuf.resize(std::min<size_t>(vtx_num-vtx_cursor,VERTEX_BUF_CAPACITY));
				vertex_t *vtx_begin=chunk->get_vertex(vtx_cursor);
				vtxbuf_iter=vtxbuf.begin();
				std::copy(vtx_begin,vtx_begin+vtxbuf.size(),vtxbuf_iter);
				//move vertex cursor forward.
			   	vtx_cursor+=vtxbuf.size();
				return;
			}
			chk_cursor++;
		}
		
		//no local chunk present.
		if (chk_cursor==chk_num){
			//invalidate vtxbuf_iter;
			vtx_num=0;
			vtx_cursor=0;
			vtxbuf_iter=vtxbuf.end();
		}

	}

	context_t *ctx;
	meta_type *meta;
	chunk_type *chunk;
	vector<vertex_t> vtxbuf;
	vector<vertex_t>::iterator vtxbuf_iter;
	size_t 	   chk_num;
	size_t     vtx_num;
	size_t     chk_cursor;
	size_t     vtx_cursor;
};

class vertex_all_iter_t:public vertex_iter_t{
public:
	vertex_all_iter_t(){

	}
	bool has_next()
	{

	}

	vertex_t* next()
	{

	}

	~vertex_all_iter_t();

};

class edge_iter_t:public iter_t<edge_t>{
public:
	explicit edge_iter_t(vertex_t vtx):vtx(vtx),edge_cursor(0)
	{
		ctx=context_t::get_context();
		meta=ctx->get_meta();
		cache=ctx->get_cache();

	}

	explicit edge_iter_t(uint64_t vtxno):edge_cursor(0)
	{

	}

	bool has_next()
	{

	}

	edge_t* next()
	{

	}

	~edge_iter_t();
private:
	context_t *ctx;
	meta_type *meta;
	chunk_type *chunk;
	cache_type *cache;
	vertex_t   vtx;

	block_t<>  blk;
	size_t 	 edge_cursor;
};

}}
#endif
