#ifndef NYNN_CLI_HPP_BY_SATANSON
#define NYNN_CLI_HPP_BY_SATANSON
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_handler.hpp>
using namespace nynn;
using namespace nynn::mm;
using namespace std;

namespace nynn{namespace cli{
class nynn_cli{
public:
	static uint32_t const firstblkno=HEAD_BLOCKNO;
	static uint32_t const lastblkno=TAIL_BLOCKNO;
	nynn_cli(string nhost,uint16_t nport,string dhost,uint16_t dport):
		_nsock(_ctx,ZMQ_REQ),_dsock(_ctx,ZMQ_REQ),_nreq(_dsock),_dreq(_nsock)
	{
		string name_endpoint="tcp://"+nhost+":"+to_string(nport);
		string data_endpoint="tcp://"+dhost+":"+to_string(dport);
		_nsock.connect(name_endpoint.c_str());
		_dsock.connect(data_endpoint.c_str());
	}
	bool read(uint32_t vtxno,uint32_t blkno,Block* blk){
		return nynn::mm::read(_dreq,vtxno,blkno,blk);
	}
	bool readfirst(uint32_t vtxno,Block* blk){
		return nynn::mm::read(_dreq,vtxno,firstblkno,blk);
	}
	bool readlast(uint32_t vtxno,Block* blk){
		return nynn::mm::read(_dreq,vtxno,lastblkno,blk);
	}
	uint32_t unshift(uint32_t vtxno,Block* blk){
		return nynn::mm::unshift(_nreq,vtxno,0,blk);
	}	
	uint32_t shift(uint32_t vtxno,Block* blk){
		return nynn::mm::shift(_nreq,vtxno,0,blk);
	}	
	uint32_t push(uint32_t vtxno,Block* blk){
		return nynn::mm::push(_nreq,vtxno,0,blk);
	}	
	uint32_t pop(uint32_t vtxno,Block* blk){
		return nynn::mm::pop(_nreq,vtxno,0,blk);
	}	
private:
	nynn_cli(nynn_cli const& rhs);
	nynn_cli& operator=(nynn_cli const& rhs);

	zmq::context_t _ctx;
	zmq::socket_t _nsock;
	zmq::socket_t _dsock;
	prot::Requester _nreq;
	prot::Requester _dreq;
};
}}
#endif
