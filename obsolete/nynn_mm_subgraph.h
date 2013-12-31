#ifndef NYNN_MM_SUBGRAPH_BY_SATANSON
#define NYNN_MM_SUBGRAPH_BY_SATANSON
#include<nynn_mm_common.h>
using namespace nynn::mm::common;
using namespace nynn::mm;
namespace nynn{namespace mm{

struct Vertex;
struct Edge;

template<
	uint32_t LOG2_BLOCKSZ,
	uint32_t LOG2_OVERFLOW_BLOCKSZ,
	uint32_t OVERFLOW_BLOCKNO_MIN,
	uint32_t BLOCKNO_MAX,
	uint32_t VERTEX_INTERVAL_WIDTH,
	uint32_t OVERFLOW_SIZE
>class SubgraphStorage;

struct Vertex{
	uint64_t m_source;
	uint64_t m_data;
	uint64_t m_nedge;
	uint32_t m_blkno;
};

struct Edge{
	time_t m_timestamp;
	uint64_t m_sink;
	union{
		void* 	 m_bval;
		uint64_t m_uval;
		uint64_t  m_ival;
		double   m_fval;
		char 	 m_cval[8];
	}m_weight;
};

/* blkno(block number)space is [0,BLOCKNO_MAX).
 * BLOCKNO_MAX<=(1<<32)-1.
 * INVALID_BLOCKNO=(1<<32)-1	 
 * [0,OVERFLOW_BLOCKNO_MIN) mapped into volume.
 * [OVERFLOW_BLOCKNO_MIN,BLOCKNO_MAX) mapped into overflow.
 *  */
template<
	uint32_t LOG2_BLOCKSZ,
	uint32_t LOG2_OVERFLOW_BLOCKSZ,
	uint32_t OVERFLOW_BLOCKNO_MIN,
	uint32_t BLOCKNO_MAX,
	uint32_t VERTEX_INTERVAL_WIDTH,
	uint32_t OVERFLOW_SIZE
>class SubgraphStorage{
public:
	static uint32_t const INVALID_BLOCKNO=~0L;
	static uint64_t const INVALID_VERTEXNO=~0LL;
	static uint32_t const BLOCKSZ=1<<LOG2_BLOCKSZ;
	static uint32_t const OVERFLOW_BLOCKSZ=1<<LOG2_OVERFLOW_BLOCKSZ;
	static uint32_t const OVERFLOW_BLOCKNO_MASK=(1<<LOG2_OVERFLOW_BLOCKSZ-LOG2_BLOCKSZ)-1;
	static uint32_t const VOLUME_SIZE=OVERFLOW_BLOCKNO_MIN*BLOCKSZ;

	union Block
	{
		char     m_data[BLOCKSZ];
		uint32_t m_indexes[BLOCKSZ/sizeof(uint32_t)];

		struct
		{
			uint32_t m_prev;
			uint32_t m_next;	
			time_t   m_infts;
			time_t   m_supts;
			uint32_t m_nedges;
		}m_header;

		static uint32_t getMaxNumberOfEdges() 
		{
			return (sizeof(Block)-sizeof(m_header))/sizeof(Edge); 
		}
		static uint32_t getMaxNumberOfIndexes()
		{
			return sizeof(m_indexes)/sizeof(m_indexes[0])-1;
		}

		bool isAtBottom() { return m_indexes[0]==1; }
		uint32_t isAtTop() { return m_indexes[0]==getMaxNumberOfIndexes(); }
		void initIndexBlock() { m_indexes[0]=0; }

		void pushIndex(uint32_t blkno) { m_indexes[++m_indexes[0]]=blkno; }		
		void setPrev(uint32_t blkno) { m_header.m_prev=blkno; }
		void setNext(uint32_t blkno) { m_header.m_next=blkno; }
		void setInfTimestamp(time_t t) { m_header.m_infts=t; }
		void setSupTimestamp(time_t t) { m_header.m_supts=t; }
		void setNumberOfEdges(uint32_t n) { m_header.m_nedges=n; }

		uint32_t popIndex() { return  m_indexes[m_indexes[0]--]; }		
		uint32_t getPrev() { return  m_header.m_prev; }
		uint32_t getNext() { return  m_header.m_next; }
		time_t getInfTimestamp() { return  m_header.m_infts; }
		time_t getSupTimeStamp() { return  m_header.m_supts; }
		uint32_t getNumberOfEdges() { return  m_header.m_nedges; }

		void setHeader(
			uint32_t prev,uint32_t next,time_t infts,time_t supts,uint32_t n)
		{
			setPrev(prev);
			setNext(next);
			setInfTimestamp(infts);
			setSupTimestamp(supts);
			setNumberOfEdges(n);
		}
		Edge* getFirstEdge() 
		{
			return reinterpret_cast<Edge*>(m_data+sizeof(m_header));
		}
		void  setEdge(uint32_t i,Edge *e) { *(getFirstEdge()+i)=*e; } 
		Edge* getEdge(uint32_t i) { return getFirstEdge()+i;}
	};

	struct SuperBlock
	{ 
		struct Data{
			uint32_t m_numOfFreeBlks;
			uint32_t m_headBlkno;
			uint32_t m_supBlkno;
			Vertex  m_vertices[VERTEX_INTERVAL_WIDTH];
		}*m_data;

		explicit SuperBlock(void *superblk=NULL) { loadData(superblk);}
		static uint32_t getSize() { return sizeof(Data);}
		void     loadData(void*superblk) { m_data=static_cast<Data*>(superblk); }
		uint32_t getSupBlkno()const { return m_data->m_supBlkno; }
		void     setSupBlkno(uint32_t blkno){ m_data->m_supBlkno=blkno; }
		uint32_t getHeadBlkno()const { return m_data->m_headBlkno; }
		void     setHeadBlkno(uint32_t blkno) { m_data->m_headBlkno=blkno; }
		void     resetNumberOfFreeBlks() { m_data->m_numOfFreeBlks=0; }
		void     incNumberOfFreeBlks() { m_data->m_numOfFreeBlks++; }
		void     decNumberOfFreeBlks() { m_data->m_numOfFreeBlks--; }
		uint32_t getNumberOfFreeBlks() { return m_data->m_numOfFreeBlks;}
	};

	struct Overflow{
		struct OverflowEntry
		{
			uint32_t m_overflowBlkno;
			uint32_t m_hit;
			void*    m_overflowBlk;
			OverflowEntry(uint32_t no=INVALID_BLOCKNO,void* addr=0):
				m_overflowBlkno(no),m_hit(0),m_overflowBlk(addr){}
		};
		vector<OverflowEntry> m_table;

		Overflow(uint32_t size=0):m_table(size){ }
		uint32_t getSize() { return m_table.size(); } 
		void setSize(uint32_t size) { m_table.resize(size); }

		uint32_t mapping(uint32_t x)
		{
			uint32_t y=x;
			y  -= OVERFLOW_BLOCKNO_MIN;
			y >>= LOG2_OVERFLOW_BLOCKSZ-LOG2_BLOCKSZ;
			y  %= getSize();
			return y;
		}	
		void* get(uint32_t blkno)
		{
			uint32_t idx=mapping(blkno);
			if (m_table[idx].m_overflowBlkno==blkno) {
				m_table[idx].m_hit++;
				return m_table[idx].m_overflowBlk; 
			}
			
			return NULL;
		}
		void set(uint32_t blkno,void*newOverflowBlk)
		{
			uint32_t idx=mapping(blkno);
			void* oldOverflowBlk=m_table[idx].m_overflowBlk;
			if (oldOverflowBlk!=NULL){
				munmap(oldOverflowBlk,OVERFLOW_BLOCKSZ);
			}
			m_table[idx].m_overflowBlkno=blkno;
			m_table[idx].m_overflowBlk=newOverflowBlk;
			m_table[idx].m_hit=0;
		}
	};


//------------------------------------------------------------------------------
	SubgraphStorage(const string &basedir):m_basedir(basedir)
	{
		try{
			string superblkPath=basedir+"/superblock.dat";
			string volumePath=basedir+"/volume.dat";
			string overflowPath=basedir+"/overflow";

			MmapFile superblkFile(superblkPath);
			MmapFile volumeFile(volumePath);
			// initialize superblock.
			m_superblk.loadData(superblkFile.getBaseAddress());

			// initialize overflow.
			glob_t g;
			g.gl_offs=0;
			string overflowBlkPathPattern=overflowPath+"/block*.dat";

			int retcode=glob(overflowBlkPathPattern.c_str(),0,0,&g);
			if (retcode!=0 && retcode!=GLOB_NOMATCH){
				string errinfo=string("Fail to invoke glob!")+"("+strerr(errno)+")";
				throwNynnException(errinfo.c_str());
			}
			m_overflow.setSize(OVERFLOW_BLOCKSZ);
			uint32_t numOfPreloadedOverflowBlk=OVERFLOW_SIZE<g.gl_pathc?
				                               OVERFLOW_SIZE:g.gl_pathc;
			for (uint32_t i=0;i<numOfPreloadedOverflowBlk;i++){
				const char* path=g.gl_pathv[i];
				const char* strBlkno=path+strlen(path)-strlen("0xffff0000.dat");
				uint32_t blkno=strtoul(strBlkno,NULL,16);

				MmapFile overflowBlkFile(path);
				setOverflowBlock(blkno,overflowBlkFile.getBaseAddress());
			}
			globfree(&g);
			// initialize volume. 
			m_volume = volumeFile.getBaseAddress();
		}catch(NynnException &err){
			throwNynnException("Fail to construct SubgraphStorage object!");
		}
	}

	Block*  getBlock(uint32_t blkno)
	{
		if (isFlat(blkno)) {
			return getVolumeBlock(blkno);
		}else if (isOverflow(blkno) && blkno<m_superblk.getSupBlkno() ){
			void *overflowBlk=getOverflowBlock(blkno);
			if (overflowBlk==NULL){
				//replace old overflow block with new one.
				MmapFile mf(getOverflowBlkPath(blkno));
				overflowBlk=mf.getBaseAddress();
				setOverflowBlock(blkno,overflowBlk);
			}
			return static_cast<Block*>(overflowBlk)+(blkno&OVERFLOW_BLOCKNO_MASK);
		}else {
			return NULL;
		}
	}

	uint32_t require()
	{
		uint32_t blkno=requireAllocated();
		return blkno==INVALID_BLOCKNO?requireUnallocated():blkno;
	}

	void release(uint32_t blkno)
	{
		uint32_t headBlkno=m_superblk.getHeadBlkno();
		Block *blk=getBlock(blkno);
		if (blk==NULL){
			throwNynnException("The released block is NULL!");
		}
		//free list is empty or its head block is full.
		if (headBlkno==INVALID_BLOCKNO||getBlock(headBlkno)->isAtTop()) {
			blk->initIndexBlock();
			blk->pushIndex(headBlkno);
			m_superblk.setHeadBlkno(blkno);
		//head block is not full.
		}else{
		 	getBlock(headBlkno)->pushIndex(blkno);
		}
		m_superblk.incNumberOfFreeBlks();
		return;
	}

	/*"SuperBlock" /path/to/basedir/superblock.dat
	 *"Volume"     /path/to/basedir/volume.dat
	 *"Overflow"   /path/to/basedir/overflow/block0xabcdabcd.dat
	 * */
	static void format(const string &path)
	{
		try{
			string superblkPath=path+"/superblock.dat";
			string volumePath=path+"/volume.dat";
			cout<<superblkPath<<"("<<SuperBlock::getSize()<<")"<<endl;
			cout<<volumePath<<"("<<VOLUME_SIZE<<")"<<endl;
			MmapFile superblkFile(superblkPath,SuperBlock::getSize());
			MmapFile volumeFile(volumePath,VOLUME_SIZE);
			string overflowDir=path+"/overflow";
			if (mkdir(overflowDir.c_str(),0770)!=0) {
				throwNynnException(strerr(errno).c_str());
			}
			void *superblk = superblkFile.getBaseAddress();
			void *volume = volumeFile.getBaseAddress();

			SubgraphStorage subgraph(path,superblk,volume);	
		}catch(NynnException &err){
			throwNynnException(err.what());
		}
	}
	template<class SubgraphStorageType>
	static void convert(const string &path,SubgraphStorageType &subgraph){
		format(path);
		SubgraphStorage s(path);
		for (uint32_t i=0;i<INVALID_BLOCKNO;i++){
			Block *
		}
	}

private:
	SubgraphStorage(const string &basedir,void* superblk,void* volume):
	m_basedir(basedir),m_superblk(superblk),m_volume(volume)
	{
		m_superblk.resetNumberOfFreeBlks();
		m_superblk.setHeadBlkno(INVALID_BLOCKNO);
		m_superblk.setSupBlkno(OVERFLOW_BLOCKNO_MIN);
		for (uint32_t i=OVERFLOW_BLOCKNO_MIN;i>0;i--)release(i-1);

		cout<<"format subgraph:"<<endl;
		cout<<"blocksz:("<<BLOCKSZ<<")"<<endl;
		cout<<"overflow_blocksz:("<<OVERFLOW_BLOCKSZ<<")"<<endl;
		cout<<"overflow_blockno_min:("<<OVERFLOW_BLOCKNO_MIN<<")"<<endl;
		cout<<"superblk:("<<SuperBlock::getSize()<<")"<<endl;
		cout<<"volume:("<<VOLUME_SIZE<<"/"<<OVERFLOW_BLOCKNO_MIN<<")"<<endl;
		cout<<"\tfree blocks:("<<m_superblk.getNumberOfFreeBlks()<<")"<<endl;
		cout<<"\thead blkno:("<<m_superblk.getHeadBlkno()<<")"<<endl;
		cout<<"\tsup blkno:("<<m_superblk.getSupBlkno()<<")"<<endl;
	}

	bool isFlat(uint32_t blkno) 
	{ 
		return 0<=blkno && blkno<=OVERFLOW_BLOCKNO_MIN; 
	}

	bool isOverflow(uint32_t blkno) 
	{ 
		return OVERFLOW_BLOCKNO_MIN<=blkno && blkno<BLOCKNO_MAX; 
	}
	
	bool isInvalid(uint32_t blkno) 
	{ 
		return BLOCKNO_MAX<=blkno<=INVALID_BLOCKNO; 
	} 

	string   getOverflowBlkPath(uint32_t blkno)
	{
		if (isOverflow(blkno)) {
			stringstream ss;
			blkno&=~OVERFLOW_BLOCKNO_MASK;
			ss<<m_basedir<<"/overflow/block0x";
			ss<<hex<<nouppercase<<setw(8)<<setfill('0');
			ss<<blkno<<".dat";
			return ss.str();
		}
		log_d("blkno=%d",blkno);
		throwNynnException("blkno is invalid or not overflow");
	}


	Block* getVolumeBlock(uint32_t blkno)
	{
		return static_cast<Block*>(m_volume)+blkno;
	}
	/* get Overflow Block.
	 * policy 1: no cache.
	 * policy 2: direct mapping cache
	 * policy 3: full associative cache
	 * policy 4: grouped mmapping cache
	 * policy 5: LRU cache
	 * policy 6: read all overflow blocks in.
	 * */
	void*  getOverflowBlock(uint32_t blkno)
	{
		return m_overflow.get(blkno&~OVERFLOW_BLOCKNO_MASK);
	}

	void   setOverflowBlock(uint32_t blkno,void* newOverflowBlk)
	{
		m_overflow.set(blkno&~OVERFLOW_BLOCKNO_MASK,newOverflowBlk);
	}

	uint32_t getOverflowSize() { return m_overflow.getSize(); };

	uint32_t requireAllocated()
	{
		uint32_t headBlkno=m_superblk.getHeadBlkno();
		//allocated space is exhausted.
		if (headBlkno == INVALID_BLOCKNO){
			return INVALID_BLOCKNO;
		}

		//head block available when reachs its bottom.
		m_superblk.decNumberOfFreeBlks();
		Block *headBlk=getBlock(headBlkno);
		if (headBlk->isAtBottom()){
			m_superblk.setHeadBlkno(getBlock(headBlkno)->popIndex());
			return headBlkno;
		}
		//head block has more than one indexes.
		return headBlk->popIndex();
	}

	uint32_t requireUnallocated()
	{
		uint32_t supBlkno=m_superblk.getSupBlkno();
		string path=getOverflowBlkPath(supBlkno);
		MmapFile mf(path.c_str(),OVERFLOW_BLOCKSZ);
		void* overflowBlk=mf.getBaseAddress();
		setOverflowBlock(supBlkno,overflowBlk);
		m_superblk.setSupBlkno(supBlkno+OVERFLOW_BLOCKNO_MASK+1);
		//insert free block into free list in reverse older.	
		for (uint32_t i=0;i<OVERFLOW_BLOCKNO_MASK+1;i++){
			release(supBlkno+OVERFLOW_BLOCKNO_MASK-i);	
		}

		return requireAllocated();
	}

private:
	string      m_basedir;
	SuperBlock  m_superblk; 
	void*       m_volume;
	Overflow    m_overflow;
};

}}
#endif

