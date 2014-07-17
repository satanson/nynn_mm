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
	nynn_fs(string na,string da)
	try:
		localhost(get_ip()),
		naddr(na),
		ncli(new nynn_ncli(ctx,naddr)),
		daddr(da)
	{
		uint32_t host=string2ip(rchop(':',daddr));
		dclimap[host].reset(new nynn_dcli(ctx,daddr));
		sgsdir=dclimap[host]->get_sgsdir();
		sgs.reset(new SubgraphSet(sgsdir));
	}catch(...){
		throw_nynn_exception(0,"failed to initializing nynn_fs");
	}
	nynn_fs(string nhost,uint16_t nport,string dhost,uint16_t dport)
	try:
		localhost(get_ip()),
		naddr(nhost+":"+to_string(nport)),
		ncli(new nynn_ncli(ctx,naddr)),
		daddr(dhost+":"+to_string(dport))
	{
		uint32_t host=string2ip(rchop(':',daddr));
		dclimap[host].reset(new nynn_dcli(ctx,daddr));
		sgsdir=dclimap[host]->get_sgsdir();
		sgs.reset(new SubgraphSet(sgsdir));
	}catch(...){
		throw_nynn_exception(0,"failed to initializing nynn_fs");
	}

	zmq::context_t& get_zmqctx(){return ctx;}
	
	string get_naddr(){return naddr;} 
	string get_daddr(){return daddr;}

	shared_ptr<SubgraphSet>& get_sgs(){return sgs;}
	shared_ptr<nynn_ncli>& get_ncli(){return ncli;}

	shared_ptr<nynn_dcli>& get_dcli(uint32_t vtxno){
		uint64_t hostport=where(vtxno);
		uint32_t host=hostport>>32;
		uint32_t port=hostport&0xffff;
		if (dclimap.count(host)==0)
			dclimap[host].reset(new nynn_dcli(ctx,ip2string(host)+":"+to_string(port)));
		return dclimap[host];
	}
	shared_ptr<nynn_dcli>& get_localdcli(){return dclimap[localhost];}
	uint32_t get_localhost()const{return localhost;}

	uint64_t where(uint32_t vtxno){
		uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
		if (dsmap.count(sgkey)==0)
			dsmap[sgkey]=dclimap[localhost]->get_remote(sgkey);
		return dsmap[sgkey];
	}
	shared_ptr<Vertex> fetchvtx(uint32_t vtxno){
		if (vtxcache.count(vtxno)==0)
			vtx_batch(get_dcli(vtxno)->get_req(),vtxno,vtxcache);
		shared_ptr<Vertex> vtx=vtxcache[vtxno];
		vtxcache.erase(vtxno);
		return vtx;
	}
	shared_ptr<Block> fetchblk(uint32_t vtxno,uint32_t blkno,uint32_t direction){
		uint64_t key=vtxnoblkno(vtxno,blkno);
		if (blkcache.count(key)==0)
			blk_batch(get_dcli(vtxno)->get_req(),vtxno,blkno,direction,blkcache);
		shared_ptr<Block> blk=blkcache[key];
		blkcache.erase(key);
		return blk;
	}
private:
	zmq::context_t ctx;
	uint32_t localhost;
	string naddr;
	string daddr;
	shared_ptr<nynn_ncli> ncli;
	unordered_map<uint32_t,shared_ptr<nynn_dcli> > dclimap;
	unordered_map<uint32_t,uint64_t> dsmap;
	unordered_map<uint32_t,shared_ptr<Vertex> > vtxcache;
	unordered_map<uint64_t,shared_ptr<Block> > blkcache;

	string sgsdir;
	shared_ptr<SubgraphSet> sgs;

	nynn_fs(nynn_fs const& fs);
	nynn_fs& operator=(nynn_fs const& fs);
};
}}
#endif
