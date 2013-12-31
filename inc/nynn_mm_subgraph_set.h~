#ifndef NYNN_MM_SUBGRAPH_SET_BY_SATANSON
#define NYNN_MM_SUBGRAPH_SET_BY_SATANSON
#include <nynn_mm_common.h>
#include <nynn_mm_subgraph_storage.h>
using namespace nynn::mm::common;
using namespace nynn::mm;
namespace nynn{namespace mm{
template<
	class SubgraphStorageT,
	class Block,
	class BlockContent,
	uint32_t VERTEX_RWLOCK_NUM
>class SubgraphSetType;

template<
	class SubgraphStorageT,
	class Block,
	class BlockContent,
	uint32_t VERTEX_RWLOCK_NUM
>class SubgraphSetType{
public:
	//very important const parameter
	static uint32_t const VERTEX_INTERVAL_WIDTH=SubgraphStorageT::VERTEX_INTERVAL_WIDTH;
	
	typedef map<uint32_t,shared_ptr<SubgraphStorageT> > SubgraphMap;
	typedef typename SubgraphMap::iterator SubgraphMapIterator;
	typedef typename Block::BlockHeader BlockHeader;

	SubgraphSetType(const string &subgraphSetBasedir):m_subgraphSetBasedir(subgraphSetBasedir)
	{
		try{
			glob_t g;
			g.gl_offs=0;
			string subgraphPathPattern=m_subgraphSetBasedir+"/subgraph0x????????";
			int retcode=glob(subgraphPathPattern.c_str(),0,0,&g);
			if (retcode!=0 && retcode!=GLOB_NOMATCH) {
				string errinfo=string("Fail to invoke glob!")+"("+strerr(errno)+")";
				throwNynnException(errinfo.c_str());
			}
			for (uint32_t  i=0;i<g.gl_pathc;i++) {
				uint32_t subgraphKey=makeSubgraphKey(g.gl_pathv[i]);
				m_subgraphMap[subgraphKey];//insert (subgraphKey,nullptr)
			}
		}catch(NynnException &ex){
			throwNynnException("Fail to create Graph object");
		}
	}

	static uint32_t const IS_WRITABLE=1;
	static uint32_t const IS_READABLE=0;
	static uint32_t const IS_NONBLOCKING=2;
	static uint32_t const IS_BLOCKING=0;
	
	void createSubgraph(uint32_t subgraphKey)
	{
		string subgraphBasedir=makeSubgraphPath(subgraphKey);
		string cmd="mkdir -p "+subgraphBasedir;
		if (system(cmd.c_str())==-1){
			string info="Fail to excecute '"+cmd+"' by invoking 'system'!";
			throwNynnException(info.c_str());
		}
		try{
			SubgraphStorageT::format(subgraphBasedir);
		}catch(NynnException &err){
			throwNynnException("Fail to format a new Subgraph!");
		}
	}

	void destroySubgraph(uint32_t subgraphKey)
	{

		string subgraphBasedir=makeSubgraphPath(subgraphKey);
		string cmd="rm -fr "+subgraphBasedir;
		if (system(cmd.c_str())==-1){
			string info="Fail to excecute '"+cmd+"' by invoking 'system'!";
			throwNynnException(info.c_str());
		}
	}

	void detachSubgraph(uint32_t subgraphKey)
	{
		ExclusiveSynchronization es(&m_subgraphMapRWLock);
		if (m_subgraphMap.find(subgraphKey)!=m_subgraphMap.end()){
			std::swap(shared_ptr<SubgraphStorageT>(),m_subgraphMap[subgraphKey]);
		}else{
			throwNynnException("The subgraph that would be detached is not found!");
		}
	}

	void attachSubgraph(uint32_t subgraphKey)
	{
		ExclusiveSynchronization es(&m_subgraphMapRWLock);
		try{
			string subgraphBasedir=makeSubgraphPath(subgraphKey);
			if (m_subgraphMap.find(subgraphKey)==m_subgraphMap.end()){
				m_subgraphMap[subgraphKey].reset(new SubgraphStorageT(subgraphBasedir));
			}
		}catch(NynnException &err){
			throwNynnException("Fail to attach specified subgraph");
		}
	}
	
	shared_ptr<SubgraphStorageT>& getSubgraph(uint32_t vtxno)
	{
		SharedSynchronization ss(&m_subgraphMapRWLock);
		uint32_t subgraphKey=VTXNO2SUBGRAPH(vtxno);
		if (m_subgraphMap.find(subgraphKey)!=m_subgraphMap.end()){
			shared_ptr<SubgraphStorageT>& subgraph=m_subgraphMap[subgraphKey];
			if (subgraph.get()!=NULL){
				return subgraph;
			}else{
				log_w("subgraph(subgraphKey=0x%08x) has not been attached!",subgraphKey);
				throwNynnException("Cannot get detached subgraph!");
			}
		}else {
			log_w("Cannot find specified subgraph(vtxno=0x%08x,subgraphKey=0x%08x)",vtxno,subgraphKey);
			throwNynnException("Cannot find specified subgraph");
		}
	}

	void getSubgraphKeys(vector<int32_t> &keys)
	{
		ExclusiveSynchronization es();
		keys.resize(0);
		keys.reserve(m_subgraphMap.size());
		for(SubgraphMapIterator si=m_subgraphMap.begin();si!=m_subgraphMap.end();si++){
			keys.push_back(si->first);
		}
	}

	uint32_t getWidthOfVertexInterval(){ return VERTEX_INTERVAL_WIDTH; }

	//bit0: 0(read).1(read&write)
	//bit1: 0(blocking).1(nonblocking).
	bool lock(uint32_t vtxno,uint32_t flag)
	{

		pthread_rwlock_t *rwlock=m_vertexRWLocks[VTXNO2RWLOCK(vtxno)].get();

		if (flag&IS_WRITABLE==IS_WRITABLE){
			if (flag&IS_NONBLOCKING==IS_NONBLOCKING) {
				if (pthread_rwlock_trywrlock(rwlock)!=0)return false;
			}else{
				pthread_rwlock_wrlock(rwlock);
			}
		}else{
			if (flag&IS_NONBLOCKING==IS_NONBLOCKING) {
				if (pthread_rwlock_tryrdlock(rwlock)!=0)return false;
			}else{
				pthread_rwlock_rdlock(rwlock);
			}
		}	

		return true;
	}

	void unlock(uint32_t vtxno)
	{
		pthread_rwlock_t *rwlock=m_vertexRWLocks[VTXNO2RWLOCK(vtxno)].get();
		if (pthread_rwlock_unlock(rwlock)!=0){
			throwNynnException("Fail to unlock vertex!");
		}
	}

	uint32_t getHeadBlkno(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getVertex(vtxno)->getHeadBlkno();
	}

	uint32_t getTailBlkno(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getVertex(vtxno)->getTailBlkno();	
	}

	void readAllBlknos(uint32_t vtxno,vector<int32_t>& blknos)
	{
		blknos.resize(0);
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);

		BlockHeader header;
		uint32_t no=subgraph->getVertex(vtxno)->getHeadBlkno();
		while(no!=INVALID_BLOCKNO){
			blknos.push_back(no);
			no=subgraph->readBlockHeader(no,&header)->getNext();
		}
	}

	Block* read(uint32_t vtxno,uint32_t blkno,Block *blk)	
	{
		return getSubgraph(vtxno)->readBlock(blkno,blk);
	}

	void readn(uint32_t vtxno,uint32_t blkno,int32_t n, vector<int8_t>& xblk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		uint32_t (BlockHeader::*next)() 
			= n>0? 
			  &BlockHeader::getNext
			  :
			  &BlockHeader::getPrev
			  ;

		xblk.reserve(n*sizeof(Block));
		n=abs(n);
		int32_t i=0;
		while (i<n && blkno!=INVALID_BLOCKNO){
			xblk.resize((i+1)*sizeof(Block));
			Block *blk=reinterpret_cast<Block*>(&xblk[i*sizeof(Block)]);
			subgraph->readBlock(blkno,blk);
			blkno=(blk->getHeader()->*next)();
		}
	}

	uint32_t getSize(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getVertex(vtxno)->size();
	}		

	uint32_t insertPrev(uint32_t vtxno,uint32_t nextBlkno, Block* blk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t headBlkno=vtx->getHeadBlkno();
		if (nextBlkno==headBlkno){
			return unshift(vtxno,blk);
		}else if (nextBlkno==INVALID_BLOCKNO){
			return push(vtxno,blk);
		}else{
			uint32_t blkno=subgraph->require();
			if (blkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;
			
			BlockContent *content=*blk;
			vtx->resize(vtx->size()+content->size());

			blk->getHeader()->setSource(vtxno);

			Block *nextBlk=subgraph->getBlock(nextBlkno);
			uint32_t prevBlkno=nextBlk->getHeader()->getPrev();
			nextBlk->getHeader()->setPrev(blkno);

			Block *prevBlk=subgraph->getBlock(prevBlkno);
			prevBlk->getHeader()->setNext(blkno);
			
			blk->getHeader()->setNext(nextBlkno);
			blk->getHeader()->setPrev(prevBlkno);
			subgraph->writeBlock(blkno,blk);
			return blkno;
		}
	}

	uint32_t insertNext(uint32_t vtxno,uint32_t prevBlkno,Block* blk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t tailBlkno=vtx->getTailBlkno();
		if (prevBlkno==tailBlkno){
			return push(vtxno,blk);
		}else if (prevBlkno==INVALID_BLOCKNO){
			return unshift(vtxno,blk);
		}else{
			uint32_t blkno=subgraph->require();
			if (blkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;
			
			BlockContent *content=*blk;
			vtx->resize(vtx->size()+content->size());
			
			blk->getHeader()->setSource(vtxno);

			Block *prevBlk=subgraph->getBlock(prevBlkno);
			uint32_t nextBlkno=prevBlk->getHeader()->getNext();
			prevBlk->getHeader()->setNext(blkno);

			Block *nextBlk=subgraph->getBlock(nextBlkno);
			nextBlk->getHeader()->setPrev(blkno);
			
			blk->getHeader()->setNext(nextBlkno);
			blk->getHeader()->setPrev(prevBlkno);
			subgraph->writeBlock(blkno,blk);
			return blkno;
		}
	}

	void remove(uint32_t vtxno,uint32_t blkno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t headBlkno=vtx->getHeadBlkno();
		uint32_t tailBlkno=vtx->getTailBlkno();

	    //if Vertex has no blocks.
		//tailBlkno==INVALID_BLOCKNO also is TRUE
		if (headBlkno==INVALID_BLOCKNO)return;

		//remove head block.
		if (headBlkno==blkno){
			shift(vtxno);
		//remove tail block.
		}else if (tailBlkno==blkno){
			pop(vtxno);
		//remove inner block that has prev or next block.
		}else{
			Block *blk=subgraph->getBlock(blkno);
			BlockContent *content=*blk;
			vtx->resize(vtx->size()-content->size());

			uint32_t prevBlkno=blk->getHeader()->getPrev();
			uint32_t nextBlkno=blk->getHeader()->getNext();
			subgraph->getBlock(prevBlkno)->getHeader()->setNext(nextBlkno);
			subgraph->getBlock(nextBlkno)->getHeader()->setPrev(prevBlkno);
			subgraph->release(blkno);
		}
	}
	
	uint32_t unshift(uint32_t vtxno,Block*newHeadBlk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex* vtx=subgraph->getVertex(vtxno);
		BlockContent *newHeadBlkContent=*newHeadBlk;
	
		vtx->resize(vtx->size()+newHeadBlkContent->size());
		uint32_t newHeadBlkno=subgraph->require();
		if (newHeadBlkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;

		newHeadBlk->getHeader()->setSource(vtxno);
		uint32_t oldHeadBlkno=vtx->getHeadBlkno();
		if (oldHeadBlkno != INVALID_BLOCKNO){
			Block* oldHeadBlk=subgraph->getBlock(oldHeadBlkno);
			vtx->setHeadBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setNext(oldHeadBlkno);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
			oldHeadBlk->getHeader()->setPrev(newHeadBlkno);
		}else{
			vtx->setHeadBlkno(newHeadBlkno);
			vtx->setTailBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setNext(INVALID_BLOCKNO);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
		}
		subgraph->writeBlock(newHeadBlkno,newHeadBlk);
//		log_i("unshift vtxno=%d blkno=%d",vtxno,newHeadBlkno);/g
		return newHeadBlkno;
	}

	void shift(uint32_t vtxno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);
		uint32_t oldHeadBlkno=vtx->getHeadBlkno();

		if (oldHeadBlkno == INVALID_BLOCKNO)return;

		Block *oldHeadBlk=subgraph->getBlock(oldHeadBlkno);
		uint32_t newHeadBlkno=oldHeadBlk->getHeader()->getNext();
		BlockContent *oldHeadBlkContent=*oldHeadBlk;
		vtx->resize(vtx->size()-oldHeadBlkContent->size());


		if (newHeadBlkno != INVALID_BLOCKNO){
			Block *newHeadBlk=subgraph->getBlock(newHeadBlkno);
			vtx->setHeadBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
		}else{
			vtx->setHeadBlkno(INVALID_BLOCKNO);
			vtx->setTailBlkno(INVALID_BLOCKNO);
		}
		subgraph->release(oldHeadBlkno);
//		log_i("shift vtxno=%d blkno=%d",vtxno,oldHeadBlkno);/g
	}

	uint32_t push(uint32_t vtxno,Block*newTailBlk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex* vtx=subgraph->getVertex(vtxno);
		BlockContent *newTailBlkContent=*newTailBlk;
	
		vtx->resize(vtx->size()+newTailBlkContent->size());
		uint32_t newTailBlkno=subgraph->require();
		if (newTailBlkno==INVALID_BLOCKNO) return INVALID_BLOCKNO;

		newTailBlk->getHeader()->setSource(vtxno);
		uint32_t oldTailBlkno=vtx->getTailBlkno();
		if (oldTailBlkno != INVALID_BLOCKNO){
			Block* oldTailBlk=subgraph->getBlock(oldTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setPrev(oldTailBlkno);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
			oldTailBlk->getHeader()->setNext(newTailBlkno);
		}else{
			vtx->setHeadBlkno(newTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setPrev(INVALID_BLOCKNO);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
		}

		subgraph->writeBlock(newTailBlkno,newTailBlk);
//		log_i("push vtxno=%d blkno=%d",vtxno,newTailBlkno);/g
		return newTailBlkno;
	}

	void pop(uint32_t vtxno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);
		uint32_t oldTailBlkno=vtx->getTailBlkno();

		if (oldTailBlkno == INVALID_BLOCKNO)return;

		Block *oldTailBlk=subgraph->getBlock(oldTailBlkno);
		uint32_t newTailBlkno=oldTailBlk->getHeader()->getPrev();
		BlockContent *oldTailBlkContent=*oldTailBlk;
		vtx->resize(vtx->size()-oldTailBlkContent->size());


		if (newTailBlkno != INVALID_BLOCKNO){
			Block *newTailBlk=subgraph->getBlock(newTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
		}else{
			vtx->setTailBlkno(INVALID_BLOCKNO);
			vtx->setTailBlkno(INVALID_BLOCKNO);
		}
		subgraph->release(oldTailBlkno);
//		log_i("pop vtxno=%d blkno=%d",vtxno,oldTailBlkno);/g
	}

	string makeSubgraphPath(uint32_t vtxno)
	{
		uint32_t subgraphKey=VTXNO2SUBGRAPH(vtxno);
		stringstream ss;
		ss<<m_subgraphSetBasedir<<"/subgraph0x";
		ss<<hex<<nouppercase<<setw(8)<<setfill('0');
		ss<<subgraphKey;
		return ss.str();
	}

	uint32_t makeSubgraphKey(string subgraphPath)
	{
		const char* path=subgraphPath.c_str();
		const char* strSubgraphNo=path+strlen(path)-strlen("0xabcd0000");
		uint32_t subgraphKey=strtoul(strSubgraphNo,NULL,16);
		return subgraphKey;
	}
	static uint32_t VTXNO2SUBGRAPH(uint32_t vtxno) { return vtxno&~(VERTEX_INTERVAL_WIDTH-1); }
	static uint32_t VTXNO2RWLOCK(uint32_t vtxno){ return vtxno%VERTEX_RWLOCK_NUM;}
private:
	
	string m_subgraphSetBasedir;
	RWLock m_vertexRWLocks[VERTEX_RWLOCK_NUM];
	RWLock m_subgraphMapRWLock;
	SubgraphMap m_subgraphMap;
};

}}
#endif
