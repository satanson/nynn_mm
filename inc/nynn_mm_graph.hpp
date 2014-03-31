#ifndef NYNN_MM_GRAPH_HPP_BY_SATANSON
#define NYNN_MM_GRAPH_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_zmqprot.hpp>
#include<zmq.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;

namespace nynn{namespace mm{
class Graph{
public:
	size_t get_sgkey_num()const{
		return m_subgset.get_sgkey_num();
	}
	template <typename Iterator>
	void get_sgkeys(Iterator begin,Iterator end){
		return m_subgset.get_sgkeys(begin,end);
	}
	uint32_t unshift(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_subgset.unshift(vtxno,(Block*)blk);
	}
	uint32_t shift(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_subgset.shift(vtxno,(Block*)blk);
	}
	uint32_t push(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_subgset.push(vtxno,(Block*)blk);
	}
	uint32_t pop(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_subgset.pop(vtxno,(Block*)blk);
	}
	uint32_t which_host(uint32_t vtxno){
		uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
		if (m_shardMap.count(sgkey)==0)return 0;
		return m_shardMap[sgkey];
	}
	bool exists(uint32_t sgkey){
		return m_subgset.exists(sgkey);
	}
	void create(uint32_t sgkey){
		m_subgset.createSubgraph(sgkey);
		m_subgset.attachSubgraph(sgkey);
	}
	void* read(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_subgset.read(vtxno,blkno,(Block*)blk);
	}

	bool read_cache(uint32_t vtxno,uint32_t blkno,void*blk){
		return m_cache.read(vtxno,blkno,(Block*)blk);
	}

	void write_cache(uint32_t vtxno,uint32_t blkno,void*blk){
		m_cache.write(vtxno,blkno,(Block*)blk);
	}

	void merge_shard_table(void* data){
		ShardTable& st=*(ShardTable*)data;
		for (int i=0;i<st.length();i++){
			m_shardMap[st[i].sgkey]=st[i].ip;
		}
	}

	Graph(const string& path) :m_subgset(path){ }


private:
	Graph(const Graph&);
	Graph& operator=(const Graph&);
	
	//SubgraphSet instance
	SubgraphSet m_subgset;
	//caching remote data
	GraphCache m_cache;
	//a table for mapping subgraphsets to hosts' ips
	unordered_map<uint32_t,uint32_t> m_shardMap;
	//mutual exclusively access m_shardMap;
};
}}
#endif
