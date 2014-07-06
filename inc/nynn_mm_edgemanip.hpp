#ifndef NYNN_MM_EDGEMANIP_HPP_BY_SATANSON
#define NYNN_MM_EDGEMANIP_HPP_BY_SATANSON
#include<nynn_mm_config.hpp>
#include<nynn_common.hpp>
#include<linuxcpp.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
namespace nynn{namespace mm{
template <typename T>
class edge_manip_t{
public:
	typedef Block::TContent<T> EdgeContent;
	static uint32_t const EDGE_CONTENT_CAP=EdgeContent::CONTENT_CAPACITY;

	explicit edge_manip_t(SubgraphSet& s):sgs(s){}

	void push_edges(uint32_t vtxno,T*begin,T*end){
		Block blk,*rb;
		EdgeContent *ctt=blk;
		rb=sgs.read(vtxno,TAIL_BLOCKNO,&blk);

		if (rb!=NULL && ctt->size()<EDGE_CONTENT_CAP){
			size_t N1=end-begin;
			size_t N2=EDGE_CONTENT_CAP-ctt->size();

			if (N1<N2){
				std::copy(begin,begin+N1,ctt->end());
				ctt->resize(ctt->size()+N1);
				sgs.pop(vtxno,NULL);
				sgs.push(vtxno,&blk);
				return;
			}else{
				std::copy(begin,begin+N2,ctt->end());
				ctt->resize(ctt->size()+N2);
				sgs.pop(vtxno,NULL);
				sgs.push(vtxno,&blk);
				begin+=N2;
			}
		}
		while(begin+EDGE_CONTENT_CAP<end){
			ctt->resize(EDGE_CONTENT_CAP);
			std::copy(begin,begin+EDGE_CONTENT_CAP,ctt->begin());
			sgs.push(vtxno,&blk);
			begin+=EDGE_CONTENT_CAP;
		}
		ctt->resize(end-begin);
		std::copy(begin,end,ctt->begin());
		sgs.push(vtxno,&blk);
	}

	void unshift_edges(uint32_t vtxno,T*begin,T*end){
		Block blk,*rb;
		EdgeContent *ctt=blk;
		rb=sgs.read(vtxno,HEAD_BLOCKNO,&blk);

		if (rb!=NULL && ctt->size()<EDGE_CONTENT_CAP){
			size_t N1=end-begin;
			size_t N2=EDGE_CONTENT_CAP-ctt->size();

			if (N1<N2){
				std::copy(begin,begin+N1,ctt->end());
				ctt->resize(ctt->size()+N1);
				sgs.shift(vtxno,NULL);
				sgs.unshift(vtxno,&blk);
				return;
			}else{
				std::copy(begin,begin+N2,ctt->end());
				ctt->resize(ctt->size()+N2);
				sgs.shift(vtxno,NULL);
				sgs.unshift(vtxno,&blk);
				begin+=N2;
			}
		}
		while(begin+EDGE_CONTENT_CAP<end){
			ctt->resize(EDGE_CONTENT_CAP);
			std::copy(begin,begin+EDGE_CONTENT_CAP,ctt->begin());
			sgs.unshift(vtxno,&blk);
			begin+=EDGE_CONTENT_CAP;
		}
		ctt->resize(end-begin);
		std::copy(begin,end,ctt->begin());
		sgs.unshift(vtxno,&blk);
	}
	size_t pop_edges(uint32_t vtxno,T*begin,T*end){
		size_t N=end-begin;
		size_t n=0;
		Block blk,*rb;
		EdgeContent *ectt=blk;
		while((rb=sgs.read(vtxno,TAIL_BLOCKNO,&blk))!=NULL){
			if(ectt->size()>N){
				n+=N;
				std::copy(ectt->end()-N,ectt->end(),begin);
				ectt->resize(ectt->size()-N);
				sgs.pop(vtxno,NULL);
				sgs.push(vtxno,&blk);
				return n;
			}else{
				n+=ectt->size();
				N-=ectt->size();
				std::copy(ectt->begin(),ectt->end(),begin());
				sgs.pop(vtxno,NULL);
				begin+=ectt->size();
			}
		}
		return n;
	}
	size_t pop_edges(uint32_t vtxno,size_t N){
		size_t n=0;
		Block blk,*rb;
		EdgeContent *ectt=blk;
		while((rb=sgs.read(vtxno,TAIL_BLOCKNO,&blk))!=NULL){
			if(ectt->size()>N){
				n+=N;
				ectt->resize(ectt->size()-N);
				sgs.pop(vtxno,NULL);
				sgs.push(vtxno,&blk);
				return n;
			}else{
				n+=ectt->size();
				N-=ectt->size();
				sgs.pop(vtxno,NULL);
			}
		}
		return n;
	}
	size_t shift_edges(uint32_t vtxno,T*begin,T*end){
		size_t N=end-begin;
		size_t n=0;
		Block blk,*rb;
		EdgeContent *ectt=blk;
		while((rb=sgs.read(vtxno,HEAD_BLOCKNO,&blk))!=NULL){
			if(ectt->size()>N){
				n+=N;
				std::copy(ectt->end()-N,ectt->end(),begin);
				ectt->resize(ectt->size()-N);
				sgs.shift(vtxno,NULL);
				sgs.unshift(vtxno,&blk);
				return n;
			}else{
				n+=ectt->size();
				N-=ectt->size();
				std::copy(ectt->begin(),ectt->end(),begin());
				sgs.shift(vtxno,NULL);
				begin+=ectt->size();
			}
		}
		return n;
	}
	size_t shift_edges(uint32_t vtxno,size_t N){
		size_t n=0;
		Block blk,*rb;
		EdgeContent *ectt=blk;
		while((rb=sgs.read(vtxno,HEAD_BLOCKNO,&blk))!=NULL){
			if(ectt->size()>N){
				n+=N;
				ectt->resize(ectt->size()-N);
				sgs.shift(vtxno,NULL);
				sgs.unshift(vtxno,&blk);
				return n;
			}else{
				n+=ectt->size();
				N-=ectt->size();
				sgs.shift(vtxno,NULL);
			}
		}
		return n;
	}

	void current_head(uint32_t vtxno,uint32_t *blkno,size_t *size){
		Subgraph *sg=sgs.getSubgraph(vtxno).get();
		Vertex *vtx=sg->getVertex(vtxno);
		*blkno=vtx->getHeadBlkno();
		if (*blkno==INVALID_BLOCKNO){
			*size=0;
			return;
		}else{
			Block *blk=sg->getBlock(*blkno);
			EdgeContent *ectt=*blk;
			*size=ectt->size();
			return;
		}
	}
	void current_tail(uint32_t vtxno,uint32_t *blkno,size_t *size){
		Subgraph *sg=sgs.getSubgraph(vtxno).get();
		Vertex *vtx=sg->getVertex(vtxno);
		*blkno=vtx->getTailBlkno();
		if (*blkno==INVALID_BLOCKNO){
			*size=0;
			return;
		}else{
			Block *blk=sg->getBlock(*blkno);
			EdgeContent *ectt=*blk;
			*size=ectt->size();
			return;
		}
	}
	
	bool shift_edges_until(uint32_t vtxno,uint32_t blkno,size_t size){
		Subgraph *sg=sgs.getSubgraph(vtxno).get();
		Vertex *vtx=sg->getVertex(vtxno);

		if (blkno==INVALID_BLOCKNO){
			blkno=vtx->getTailBlkno();
			assert(size==0);
		};

		uint32_t b=vtx->getHeadBlkno();
		int k=0;
		while(b!=blkno&&b!=INVALID_BLOCKNO){
			k++;
			b=sg->getBlock(b)->getHeader()->getNext();
		}
		if (b==INVALID_BLOCKNO){
			return false;
		}else{
			while(k-->0)sgs.shift(vtxno,NULL);
			assert(blkno==b && blkno==vtx->getHeadBlkno());
			Block *blk=sg->getBlock(blkno);
			EdgeContent *ectt=*blk;
			assert(size<=ectt->size());
			ectt->resize(size);
			return true;
		}
	}
	bool pop_edges_until(uint32_t vtxno,uint32_t blkno,size_t size){
		Subgraph *sg=sgs.getSubgraph(vtxno).get();
		Vertex *vtx=sg->getVertex(vtxno);

		if (blkno==INVALID_BLOCKNO){
			blkno=vtx->getHeadBlkno();
			assert(size==0);
		};

		uint32_t b=vtx->getTailBlkno();
		int k=0;
		while(b!=blkno&&b!=INVALID_BLOCKNO){
			k++;
			b=sg->getBlock(b)->getHeader()->getPrev();
		}
		if (b==INVALID_BLOCKNO){
			return false;
		}else{
			while(k-->0)sgs.pop(vtxno,NULL);
			assert(blkno==b && blkno==vtx->getTailBlkno());
			Block *blk=sg->getBlock(blkno);
			EdgeContent *ectt=*blk;
			assert(size<=ectt->size());
			ectt->resize(size);
			return true;
		}
	}
private:
	SubgraphSet &sgs;
	edge_manip_t(const edge_manip_t&);
	edge_manip_t& operator=(const edge_manip_t&);
};
}}
#endif
