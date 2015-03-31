#ifndef NYNN_MM_TYPES_HPP_BY_SATANSON
#define NYNN_MM_TYPES_HPP_BY_SATANSON
#include<nynn_common.hpp>
using namespace nynn;

namespace nynn{namespace mm{
	
struct Vertex;
struct Edge;
template <uint32_t BLOCKSZ> union BlockType;

static uint32_t const INVALID_BLOCKNO=~0U;
static uint32_t const TAIL_BLOCKNO=~0U-1;
static uint32_t const HEAD_BLOCKNO=~0u-2;
static uint32_t const INVALID_VERTEXNO=~0U;

struct Vertex{
private:
	uint32_t m_source;
	uint32_t m_data;
	uint32_t m_size;
	uint32_t m_headBlkno;
	uint32_t m_tailBlkno;
public:
	static uint32_t const EXIST_BIT=1<<31;
	explicit Vertex(uint32_t source):
		m_source(source),m_data(0),m_size(0),
		m_headBlkno(INVALID_VERTEXNO),
		m_tailBlkno(INVALID_VERTEXNO){}

	uint32_t getSource()const{ return m_source; }
	uint32_t getData()const{ return m_data; }
	uint32_t size()const { return m_size; }
	uint32_t getHeadBlkno()const { return m_headBlkno; }
	uint32_t getTailBlkno()const { return m_tailBlkno; }

	void setSource(uint32_t source) { m_source=source; }
	void setData(uint32_t data) { m_data=data; }
	void resize(uint32_t sz) { m_size=sz; }
	void setHeadBlkno(uint32_t blkno) { m_headBlkno=blkno; }
	void setTailBlkno(uint32_t blkno) { m_tailBlkno=blkno; }

	void setExistBit(){m_data|=EXIST_BIT;}
	void resetExistBit(){m_data&=~EXIST_BIT;}
	bool getExistBit(){return (m_data&EXIST_BIT)==EXIST_BIT;}
}__attribute__((packed));
struct Edge{
	uint32_t m_sink;
	union{
		void* 	 m_bval;
		uint64_t m_uval;
		double   m_fval;
		char 	 m_cval[8];
	}m_weight;
	uint64_t m_timestamp;
}__attribute__((packed));

struct PREFETCH_POLICY {
	const static uint32_t NOP=0;
	const static uint32_t SEQ=1;
	const static uint32_t BFS=2;
	const static uint32_t DFS=3;
};

#if 0 
struct Edge{
	uint32_t m_sink;
	uint32_t m_timestamp;
	uint32_t type;
    uint32_t topic;	
}__attribute__((packed));
#endif 


template <uint32_t BLOCKSZ> 
union BlockType
{
public:
	struct RawBlock
	{
	private:
		std::vector<int8_t> m_raw;
	public:
		RawBlock():m_raw(BLOCKSZ){}
		std::vector<int8_t>& getRaw(){ return this->m_raw;}	
		BlockType* getBlock(){ return reinterpret_cast<BlockType*>(this->m_raw.data());}
		operator std::vector<int8_t>& () { return this->getRaw();}
		operator BlockType* () { return this->getBlock();}
	};

	struct BlockHeader
	{
	private:
		uint32_t m_source;
		uint32_t m_blkno;
		uint32_t m_prev;
		uint32_t m_next;	
		uint64_t   m_infts;
		uint64_t   m_supts;
	public:

		void setSource(uint32_t vtxno) { m_source=vtxno; }
		void setBlkno(uint32_t blkno){ m_blkno=blkno;}
		void setPrev(uint32_t blkno) { m_prev=blkno; }
		void setNext(uint32_t blkno) { m_next=blkno; }
		void setInfTimestamp(uint64_t t) { m_infts=t; }
		void setSupTimestamp(uint64_t t) { m_supts=t; }

		uint32_t getSource() { return m_source; }
		uint32_t getBlkno(){ return m_blkno; }
		uint32_t getPrev() { return  m_prev; }
		uint32_t getNext() { return  m_next; }
		uint64_t getInfTimestamp() { return  m_infts; }
		uint64_t getSupTimestamp() { return  m_supts; }

		bool isNewerThan(const BlockHeader *rhs)
		{ 
			return this->getInfTimestamp()>=rhs->getSupTimestamp();
		}
		bool isNewerThan(uint64_t timestamp)
		{
			return this->getInfTimestamp()>=timestamp;
		}
		bool isOlderThan(const BlockHeader *rhs)
		{
			return this->getSupTimestamp()<=rhs->getInfTimestamp();
		}
		bool isOlderThan(uint64_t timestamp)
		{
			return this->getSupTimestamp()<=timestamp;
		}

		friend ostream &operator<<(ostream& out,const BlockHeader & h)
		{
			out<<"source vertex="<<h.m_source<<endl;
			out<<"prev block="<<h.m_prev<<endl;
			out<<"next block="<<h.m_next<<endl;
			out<<"inf timestamp="<<time2str(h.m_infts)<<endl;
			out<<"sup timestamp="<<time2str(h.m_supts)<<endl;
			return out;
		}
	}__attribute__((packed));
	
	template<typename T> struct  ContentTrait {typedef T BlockContentType; };

	template <typename T>
	struct TContent:public ContentTrait<TContent<T> >
	{
		static uint32_t const BLOCK_CONTENT_SIZE=BLOCKSZ-sizeof(BlockHeader)-sizeof(uint16_t);
		static uint16_t const CONTENT_CAPACITY=BLOCK_CONTENT_SIZE/sizeof(T);
	private:
		uint16_t m_size;
		T m_array[CONTENT_CAPACITY];
	public:
		uint16_t size()const{ return m_size; };
		void  resize(uint16_t size){ m_size = size;} 

		T* begin(){ return &m_array[0]; }
		T* end(){ return &m_array[m_size]; }
		T* pos(uint16_t i){ return &m_array[i];} 
	}__attribute__((packed));

	typedef TContent<char> CharContent;
	typedef TContent<Edge> EdgeContent;

	BlockHeader* getHeader() { return &m_header;}
	void setHeader(BlockHeader*header) { m_header=*header; } 

	template <typename Content> 
	operator Content*() 
	{ 
		return reinterpret_cast<typename Content::BlockContentType*>(m_data+sizeof(m_header));
	}

	template <typename Content>
	BlockType& operator=(Content* content){
		memcpy(m_data+sizeof(m_header),content,sizeof(Content));
	}

	static uint32_t getMaxNumberOfIndexes()
	{
		return sizeof(m_indexes)/sizeof(m_indexes[0])-1;
	}
	//index block operations.
	void initIndexBlock() { m_indexes[0]=0; }
	bool isAtBottom() { return m_indexes[0]==1; }
	uint32_t isAtTop() { return m_indexes[0]==getMaxNumberOfIndexes(); }
	void pushIndex(uint32_t blkno) { m_indexes[++m_indexes[0]]=blkno; }		
	uint32_t popIndex() { return  m_indexes[m_indexes[0]--]; }		
private:
	BlockType& operator=(const BlockType& other);
	char     m_data[BLOCKSZ];
	uint32_t m_indexes[BLOCKSZ/sizeof(uint32_t)];
	BlockHeader m_header;
}__attribute__((packed));

struct GraphInfo{
	uint32_t gi_max_vtxno;
	uint32_t gi_min_vtxno;
	uint64_t gi_vtx_num;
	uint64_t gi_edge_num;
	void setMaxVtxno(uint32_t maxno){gi_max_vtxno=maxno;}
	void setMinVtxno(uint32_t minno){gi_min_vtxno=minno;}
	uint32_t getMaxVtxno(){return gi_max_vtxno;}
	uint32_t getMinVtxno(){return gi_min_vtxno;}
}__attribute__((packed));

void dealloc(void* p,void*hint){operator delete(p);}

template <typename Fixed,typename VariedElement>
struct Varied{
	Fixed fix;
	mutable uint32_t len;
	typedef Fixed FixedType;
	typedef VariedElement  VariedElemType;
	typedef VariedElement* iterator;

	static Varied* make(uint32_t n){
		n=(n<0)?0:n;
		char *p=new char[sizeof(Varied)+sizeof(VariedElement)*n];
		Varied& var=*(Varied*)p;
		var.len=n;
		return &var;
	}
	VariedElement& operator[](ssize_t i){
		i=(i<0?(ssize_t)len+i:i);
		VariedElement* base=(VariedElement*)((char*)this+sizeof(Varied));
		return *(base+i);
	}
	iterator begin(){return &(*this)[0];}
	iterator end(){return &(*this)[len];}
	Varied& augment(size_t i){len+=i; return *this;}
	Varied& shrink(size_t i){len-=i; return *this;}
	operator bool(){return len!=0;}
	size_t size(){return sizeof(Varied)+sizeof(VariedElement)*len;}
	size_t length(){return len;}
	void relength(size_t l){len=l;}
	Fixed* operator ->(){return &fix;}
};

struct WriteOptionsFixed{
	uint32_t op;
	uint32_t vtxno;
	uint32_t blkno;
	uint32_t replicas;
};
struct SubmitOptionsFixed{
	uint32_t ip;
};
struct HelloOptionsFixed{
	uint32_t ip;
};
struct STEntry{
	uint32_t sgkey;
	uint32_t ip;
};
struct JustPadding{
	uint32_t padding;
};
struct VtxPrefetchOptionsFixed{
	uint32_t vtxno;
	uint32_t prefetch;
};
struct BlkPrefetchOptionsFixed{
	uint32_t vtxno;
	uint32_t blkno;
	uint32_t direction;
	uint32_t prefetch;
};
typedef Varied<WriteOptionsFixed,uint32_t> WriteOptions;
typedef Varied<SubmitOptionsFixed,uint32_t> SubmitOptions;
typedef Varied<HelloOptionsFixed,JustPadding> HelloOptions;
typedef Varied<JustPadding,JustPadding> NotifyOptions;
typedef Varied<JustPadding,JustPadding> NullOptions;
typedef Varied<JustPadding,STEntry> ShardTable;

typedef Varied<JustPadding,char> VarString;
typedef Varied<JustPadding,uint32_t> VtxOptions;
typedef Varied<BlkPrefetchOptionsFixed,JustPadding> BlkPrefetchOptions;
typedef Varied<VtxPrefetchOptionsFixed,JustPadding> VtxPrefetchOptions;
}}
#endif
