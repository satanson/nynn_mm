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
class nynn_ncli{
public:
	nynn_ncli(zmq::context_t& ctx,string const& naddr)
	try:_ctx(ctx),_nsock(_ctx,ZMQ_REQ),_nreq(_nsock)
	{
		string protocol=naddr.substr(0,3);
		if (likely(protocol=="ipc")){
			_nsock.connect(naddr.c_str());
		} else{
			string name_endpoint="tcp://"+naddr;
			_nsock.connect(name_endpoint.c_str());
		}
	}catch(...){
		throw_nynn_exception(0,"failed to initializing name server client");
	}

	nynn_ncli(zmq::context_t& ctx,string nhost,uint16_t nport)
	try:_ctx(ctx),_nsock(_ctx,ZMQ_REQ),_nreq(_nsock)
	{
		string name_endpoint="tcp://"+nhost+":"+to_string(nport);
		_nsock.connect(name_endpoint.c_str());
	}catch(...){
		throw_nynn_exception(0,"failed to initializing name server client");
	}
	
	uint32_t unshift(uint32_t vtxno,Block* blk){
		Synchronization get(&_nlock);
		return nynn::mm::unshift(_nreq,vtxno,0,blk);
	}	
	uint32_t shift(uint32_t vtxno,Block* blk){
		Synchronization get(&_nlock);
		return nynn::mm::shift(_nreq,vtxno,0,blk);
	}	
	uint32_t push(uint32_t vtxno,Block* blk){
		Synchronization get(&_nlock);
		return nynn::mm::push(_nreq,vtxno,0,blk);
	}	
	uint32_t pop(uint32_t vtxno,Block* blk){
		Synchronization get(&_nlock);
		return nynn::mm::pop(_nreq,vtxno,0,blk);
	}	
private:
	nynn_ncli(nynn_ncli const& rhs);
	nynn_ncli& operator=(nynn_ncli const& rhs);

	zmq::context_t& _ctx;
	zmq::socket_t _nsock;
	prot::Requester _nreq;
	Monitor _nlock;
};

class nynn_dcli{
public:

	nynn_dcli(zmq::context_t& ctx,string const& daddr)
	try:_ctx(ctx),_dsock(_ctx,ZMQ_REQ),_dreq(_dsock)
	{
		string data_endpoint="tcp://"+daddr;
		//log_i("conntect to %s",data_endpoint.c_str());
		_dsock.connect(data_endpoint.c_str());
	}catch(...){
		throw_nynn_exception(0,"failed to initializing data server client");
	}

	nynn_dcli(zmq::context_t& ctx,string dhost,uint16_t dport)
	try:_ctx(ctx),_dsock(_ctx,ZMQ_REQ),_dreq(_dsock)
	{
		string data_endpoint="tcp://"+dhost+":"+to_string(dport);
		_dsock.connect(data_endpoint.c_str());
	}catch(...){
		throw_nynn_exception(0,"failed to initializing data server client");
	}
	
	bool read(uint32_t vtxno,uint32_t blkno,Block* blk){
		//Synchronization get(&_dlock);
		return nynn::mm::read(_dreq,vtxno,blkno,blk);
	}
	bool vtx_exists(uint32_t vtxno){
		//Synchronization get(&_dlock);
		return nynn::mm::vtx_exists(_dreq,vtxno);
	}
	string get_sgsdir(){
		//Synchronization get(&_dlock);
		return nynn::mm::get_sgsdir(_dreq);
	}
	uint64_t get_remote(uint32_t vtxno){
		//Synchronization get(&_dlock);
		return nynn::mm::get_remote(_dreq,vtxno);
	}
  	prot::Requester& get_req(){return _dreq;}
private:
	nynn_dcli(nynn_dcli const& rhs);
	nynn_dcli& operator=(nynn_dcli const& rhs);

	zmq::context_t& _ctx;
	zmq::socket_t _dsock;
	prot::Requester _dreq;
	Monitor _dlock;
};
}}
#endif
