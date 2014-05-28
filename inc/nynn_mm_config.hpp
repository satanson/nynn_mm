#ifndef NYNN_MM_CONFIG_HPP_BY_SATANSON
#define NYNN_MM_CONFIG_HPP_BY_SATANSON
#include<nynn_mm_types.hpp>
#include<nynn_mm_subgraph_storage.hpp>
#include<nynn_mm_subgraph_set.hpp>
#include<nynn_mm_graph_cache.hpp>
using namespace nynn::mm;
typedef nynn::mm::SubgraphStorageType<16,19,1<<4,~0u,10,1<<2,64> Subgraph;
typedef Subgraph::Block Block;
typedef Block::TContent<char> CharContent;
typedef Block::RawBlock RawBlock;
typedef nynn::mm::SubgraphSetType<Subgraph,Block,CharContent,64> SubgraphSet;
typedef nynn::mm::GraphCacheType<16,1024*128,1024,64> GraphCache;
enum{
	IS_WRITABLE=SubgraphSet::IS_WRITABLE,
	IS_READABLE=SubgraphSet::IS_READABLE,
	IS_NONBLOCKING=SubgraphSet::IS_NONBLOCKING,
	IS_BLOCKING=SubgraphSet::IS_BLOCKING
};
#endif
