#ifndef NYNN_MM_SUBGRAPH_CACHE_HPP_BY_SATANSON
#define NYNN_MM_SUBGRAPH_CACHE_HPP_BY_SATANSON
#include<nynn_common.hpp>
#include<nynn_mm_subgraph_storage.hpp>
#include<nynn_mm_types.hpp>

using namespace nynn;
namespace nynn{namespace mm{
template <
	uint32_t LOG2_BLOCKSZ,
	uint32_t CACHE_BLOCK_NUM,
	uint32_t CACHE_HEAD_NUM,
	uint32_t MONITOR_NUM
> class GraphCacheType;

	
template <
	uint32_t LOG2_BLOCKSZ,
	uint32_t CACHE_BLOCK_NUM,
	uint32_t CACHE_HEAD_NUM,
	uint32_t MONITOR_NUM
>
class GraphCacheType{
public:
	typedef nynn::mm::BlockType<1<<LOG2_BLOCKSZ> Block;

	static uint32_t const LRU_WINDOW_SIZE=CACHE_BLOCK_NUM/CACHE_HEAD_NUM;

	struct LRUListEntry{
		uint32_t m_vtxno;
		uint32_t m_blkno;
		uint32_t m_next;
		uint32_t m_prev;
	};
	
	struct LRUList{
		uint32_t m_next;
		uint32_t m_prev;
		uint32_t m_num;
	};

	static uint32_t hash(uint32_t vtxno,uint32_t blkno)
	{
		vtxno+=blkno;
		return rand_r(&vtxno)%CACHE_HEAD_NUM;
	}
	
	Block* read(uint32_t vtxno,uint32_t blkno,Block *blk)
	{
		assert(vtxno!=INVALID_VERTEXNO);
		assert(blkno!=INVALID_BLOCKNO);
		
		uint32_t h=hash(vtxno,blkno);
		Synchronization s(&m_LRUListMonitors[h%MONITOR_NUM]);
		uint32_t e=lookup(h,vtxno,blkno);
		
		if (e==INVALID_BLOCKNO)return NULL;

		move_front(h,e);
		memcpy(blk,m_data+e,sizeof(Block));
		return blk;
	}

	void write(uint32_t vtxno,uint32_t blkno,Block *blk)
	{

		assert(vtxno!=INVALID_VERTEXNO);
		assert(blkno!=INVALID_BLOCKNO);

		uint32_t h=hash(vtxno,blkno);
		Synchronization s(&m_LRUListMonitors[h%MONITOR_NUM]);
		uint32_t e=lookup(h,vtxno,blkno);

		//not cached yet.
		if (e==INVALID_BLOCKNO){			
			//create a new one
			if (m_LRUTable[h].m_num<LRU_WINDOW_SIZE){
				e=popFreeEntry();
				//LRU_WINDOW_SIZE*CACHE_HEAD_NUM=CACHE_BLOCK_NUM ensure following.
				assert(e!=INVALID_BLOCKNO);
				push_front(h,e);
				insertIndex(vtxno,blkno,e);
			//subsitition
			}else{
				e=m_LRUTable[h].m_prev;
				eraseIndex(m_entries[e].m_vtxno,m_entries[e].m_blkno);
				move_front(h,e);
				insertIndex(vtxno,blkno,e);
				
			}
		//already cached.	
		}else{
			move_front(h,e);
		}
		m_entries[e].m_vtxno=vtxno;
		m_entries[e].m_blkno=blkno;
		memcpy(m_data+e,blk,sizeof(Block));
	}

	GraphCacheType()
	{
		memset(&m_data,0,sizeof(m_data));
		m_freeList.m_next=INVALID_BLOCKNO;
		m_freeList.m_prev=INVALID_BLOCKNO;
		m_freeList.m_num=0;
		for (uint32_t i=0;i<CACHE_BLOCK_NUM;i++){
			uint32_t k=CACHE_BLOCK_NUM-1-i;
			m_entries[k].m_next=m_freeList.m_next;
			m_freeList.m_next=k;
			m_freeList.m_num++;

			m_entries[k].m_vtxno=INVALID_VERTEXNO;
			m_entries[k].m_blkno=INVALID_BLOCKNO;
			m_entries[k].m_prev=INVALID_BLOCKNO;
		}
		for (uint32_t h=0;h<CACHE_HEAD_NUM;h++) {
			m_LRUTable[h].m_prev=INVALID_BLOCKNO;
			m_LRUTable[h].m_next=INVALID_BLOCKNO;
			m_LRUTable[h].m_num=0;
		}
	}
	~GraphCacheType()
	{
	}

private:

	GraphCacheType(const GraphCacheType&);
	GraphCacheType& operator=(const GraphCacheType&);

	uint32_t lookup(uint32_t h,uint32_t vtxno,uint32_t blkno){
		string key=to_string(vtxno)+"#"+to_string(blkno);
		if (m_index.count(key)!=0){
			uint32_t e=m_index[key];
			if (m_entries[e].m_vtxno==vtxno&&m_entries[e].m_blkno==blkno)
				return e;
			else
				return INVALID_BLOCKNO;
		}else{
			return INVALID_BLOCKNO;
		}
	}
	void insertIndex(uint32_t vtxno,uint32_t blkno,uint32_t e ){
		string key=to_string(vtxno)+"#"+to_string(blkno);
		m_index[key]=e;
	}

	void eraseIndex(uint32_t vtxno,uint32_t blkno){
		string key=to_string(vtxno)+"#"+to_string(blkno);
		m_index.erase(key);
	}



	uint32_t popFreeEntry()
	{ 			
		Synchronization s(&m_freeListMonitor);
		uint32_t curr=m_freeList.m_next;
		m_freeList.m_next=m_entries[curr].m_next;
		m_freeList.m_num--;
		return curr;
	}

	void checkLRUList(uint32_t h)
	{
#ifdef DEBUG
		LRUList &head = m_LRUTable[h];
		assert(
				(head.m_next!=INVALID_BLOCKNO&&
				m_entries[head.m_next].m_prev==INVALID_BLOCKNO)
				||
				(head.m_prev!=INVALID_BLOCKNO&&
				m_entries[head.m_prev].m_next==INVALID_BLOCKNO)
		);
#endif
	}

	void move_front(uint32_t h,uint32_t e)
	{
		assert(e!=INVALID_BLOCKNO);
		LRUList &head=m_LRUTable[h];
		//move current entry to head if it's not at head.
		if (head.m_next!=e){
			//remove the entry
			//e must have a m_prev entry.make e's m_prev'm_next point to e'm_next;
			m_entries[m_entries[e].m_prev].m_next=m_entries[e].m_next;
			//if e have a non-null m_next entry,
			//make e's m_next'm_prev point to e's m_prev.
			if (m_entries[e].m_next!=INVALID_BLOCKNO){
				assert(head.m_prev!=e);
				m_entries[m_entries[e].m_next].m_prev=m_entries[e].m_prev;
			
			//if e is the tail,make e's m_prev to be a new tail.
			}else{
				assert(head.m_prev==e);
				head.m_prev=m_entries[e].m_prev;
				m_entries[head.m_prev].m_next=INVALID_BLOCKNO;
			}

			//insert the entry at head
			m_entries[e].m_prev=INVALID_BLOCKNO;
			m_entries[e].m_next=head.m_next;
			m_entries[head.m_next].m_prev=e;
			head.m_next=e;
		}
		checkLRUList(h);
	}

	void push_front(uint32_t h,uint32_t e)
	{
		LRUList &head=m_LRUTable[h];
		assert(head.m_num<LRU_WINDOW_SIZE);
		assert(e!=INVALID_BLOCKNO);

		//insert e at head.
		m_entries[e].m_next=head.m_next;
		head.m_next=e;

		//if e is not only entry in list, so make e be its m_next entry'm_prev entry.
		if (m_entries[e].m_next!=INVALID_BLOCKNO){
			assert(head.m_prev!=INVALID_BLOCKNO);
			m_entries[m_entries[e].m_next].m_prev=e;
		
		//e as the only entry in list,so e also is tail.
		}else{
			assert(head.m_prev==INVALID_BLOCKNO);
			head.m_prev=e;
		}
		m_entries[e].m_prev=INVALID_BLOCKNO;
		head.m_num++;
		checkLRUList(h);
	}

	LRUList  m_LRUTable[CACHE_HEAD_NUM];
	LRUList  m_freeList;
	LRUListEntry m_entries[CACHE_BLOCK_NUM];
	Block m_data[CACHE_BLOCK_NUM];
	unordered_map<string,uint32_t> m_index;

	Monitor m_freeListMonitor;
	Monitor m_LRUListMonitors[MONITOR_NUM];
};

}}
#endif
