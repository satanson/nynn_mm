#ifndef NYNN_MM_TYPES_H_BY_SATANSON
#define NYNN_MM_TYPES_H_BY_SATANSON
#include<nynn_mm_common.h>
using namespace nynn::mm::common;

namespace nynn{namespace mm{
	
struct Vertex;
struct Edge;
template <uint32_t BLOCKSZ> union BlockType;

static uint32_t const INVALID_BLOCKNO=~0L;
static uint32_t const INVALID_VERTEXNO=~0L;

struct Vertex{
private:
	uint32_t m_source;
	uint32_t m_data;
	uint32_t m_size;
	uint32_t m_headBlkno;
	uint32_t m_tailBlkno;
public:
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
};

struct Edge{
	time_t m_timestamp;
	uint32_t m_sink;
	union{
		void* 	 m_bval;
		uint64_t m_uval;
		double   m_fval;
		char 	 m_cval[8];
	}m_weight;
};

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
		uint32_t m_prev;
		uint32_t m_next;	
		time_t   m_infts;
		time_t   m_supts;
	public:

		void setSource(uint32_t vtxno) { m_source=vtxno; }
		void setPrev(uint32_t blkno) { m_prev=blkno; }
		void setNext(uint32_t blkno) { m_next=blkno; }
		void setInfTimestamp(time_t t) { m_infts=t; }
		void setSupTimestamp(time_t t) { m_supts=t; }

		uint32_t getSource() { return m_source; }
		uint32_t getPrev() { return  m_prev; }
		uint32_t getNext() { return  m_next; }
		time_t getInfTimestamp() { return  m_infts; }
		time_t getSupTimestamp() { return  m_supts; }

		bool isNewerThan(const BlockHeader *rhs)
		{ 
			return this->getInfTimestamp()>=rhs->getSupTimestamp();
		}
		bool isNewerThan(time_t timestamp)
		{
			return this->getInfTimestamp()>=timestamp;
		}
		bool isOlderThan(const BlockHeader *rhs)
		{
			return this->getSupTimestamp()<=rhs->getInfTimestamp();
		}
		bool isOlderThan(time_t timestamp)
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

}}
#endif
