#ifndef NYNN_MM_CONFIG_H_BY_SATANSON
#define NYNN_MM_CONFIG_H_BY_SATANSON
#include<nynn_mm_types.h>
#include<nynn_mm_subgraph_storage.h>
#include<nynn_mm_subgraph_set.h>
#include<nynn_mm_graph_cache.h>
using namespace nynn::mm;
typedef nynn::mm::SubgraphStorageType<9,12,8,~0L,3,1<<10,64> Subgraph;
typedef Subgraph::Block Block;
typedef Block::TContent<char> CharContent;
typedef Block::RawBlock RawBlock;
typedef nynn::mm::SubgraphSetType<Subgraph,Block,CharContent,64> SubgraphSet;
typedef nynn::mm::GraphCacheType<9,1024*64,1024,64> GraphCache;
enum{
	IS_WRITABLE=SubgraphSet::IS_WRITABLE,
	IS_READABLE=SubgraphSet::IS_READABLE,
	IS_NONBLOCKING=SubgraphSet::IS_NONBLOCKING,
	IS_BLOCKING=SubgraphSet::IS_BLOCKING
};
#endif
