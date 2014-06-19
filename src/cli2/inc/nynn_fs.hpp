#ifndef NYNN_FS_HPP_BY_SATANSON
#define NYNN_FS_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_zmq.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_cli.hpp>
using namespace std;
using namespace nynn::mm;
using namespace nynn::cli;
namespace nynn{namespace cli{
class nynn_fs{
public:
	nynn_fs(string naddr,string daddr)
	try:
		_naddr(naddr),
		_daddr(daddr),
		_ncli(_ctx,_naddr),
		_dcli(_ctx,_daddr),
		_sgsdir(_dcli.get_sgsdir()),
		_sgs(_sgsdir)
	{
	}catch(...){
		throw_nynn_exception(0,"failed to initializing nynn_fs");
	}
	nynn_fs(string nhost,uint16_t nport,string dhost,uint16_t dport)
	try:
		_naddr(nhost+":"+to_string(nport)),
		_daddr(dhost+":"+to_string(dport)),
		_ncli(_ctx,_naddr),
		_dcli(_ctx,_daddr),
		_sgsdir(_dcli.get_sgsdir()),
		_sgs(_sgsdir)
	{
	}catch(...){
		throw_nynn_exception(0,"failed to initializing nynn_fs");
	}

	//nynn_file get_file(uint32_t vtxno,bool writable=false){
	//	return nynn_file(*this,vtxno,writable);
	//}
	zmq::context_t& get_zmqctx(){return _ctx;}
	
	string get_naddr(){return _naddr;} 
	string get_daddr(){return _daddr;}

	SubgraphSet& get_sgs(){return _sgs;}
	nynn_ncli& get_ncli(){return _ncli;}
	nynn_dcli& get_dcli(){return _dcli;}
private:
	zmq::context_t _ctx;

	string _naddr;
	string _daddr;
	nynn_ncli _ncli;
	nynn_dcli _dcli;

	string _sgsdir;
	SubgraphSet _sgs;

	nynn_fs(nynn_fs const& fs);
	nynn_fs& operator=(nynn_fs const& fs);

};
}}
#endif
