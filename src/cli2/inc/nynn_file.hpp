#ifndef NYNN_FILE_HPP_BY_SATANSON
#define NYNN_FILE_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_fs.hpp>
using namespace std;
using namespace nynn::mm;
using namespace nynn::cli;
namespace nynn{namespace cli{
class nynn_file{
public:
	static const uint32_t  invalidblkno=INVALID_BLOCKNO;
	static const uint32_t headblkno=HEAD_BLOCKNO;
	static const uint32_t tailblkno=TAIL_BLOCKNO;
	nynn_file(nynn_fs& fs,uint32_t vtxno,bool writable=false)
		:_fs(fs),_vtxno(vtxno),_writable(writable)
	{
		try{
		if(likely(!_writable)){
			//log_i("vtxno=%d readonly",vtxno);
			uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
			if (likely(_fs.get_sgs().exists(sgkey))){
				//log_i("vtxno=%d in local",vtxno);
				_local=true;
			}else{
				string remote=_fs.get_dcli().get_remote(vtxno);
				//log_i("vtxno=%d in remote %s",vtxno,remote.c_str());
				string host=rchop(':',remote);
				uint32_t remoteip=string2ip(host);
				if (unlikely(remoteip==0)){
					throw_nynn_exception(0,format("%d not exists!",vtxno).c_str());
				}else if (remoteip==get_ip()){
					//log_i("vtxno=%d actually in local",vtxno);
					_local=true;
					uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
					_fs.get_sgs().attachSubgraph(sgkey);
				}else{
					//log_i("vtxno=%d required from %s",vtxno,_fs.get_daddr().c_str());
					_local=false;
					//_dcli_ptr.reset(new nynn_dcli(_fs.get_zmqctx(),_fs.get_daddr()));
					//log_i("vtxno=%d required from %s",vtxno,remote.c_str());
					_dcli_ptr.reset(new nynn_dcli(_fs.get_zmqctx(),remote));
				}
			}
		}else{
			//log_i("vtxno=%d writable",vtxno);
			//log_i("vtxno=%d nameserv=%s",_fs.get_naddr().c_str());
			//log_i("vtxno=%d dataserv=%s",_fs.get_daddr().c_str());
			string naddr=_fs.get_naddr();
			uint32_t nhost=string2ip(rchop(':',naddr));
			string nport=lchop(':',naddr);
			if (likely(nhost==get_ip()))naddr=string("ipc:///var/nynn/nameserv")+"."+nport;
			_ncli_ptr.reset(new nynn_ncli(_fs.get_zmqctx(),naddr));
			_dcli_ptr.reset(new nynn_dcli(_fs.get_zmqctx(),_fs.get_daddr()));
		}
		}catch(exception& err){
			cerr<<err.what()<<endl;
			throw_nynn_exception(0,format("fail to initialize nynn_file specified by %d",vtxno).c_str());
		}
	}
	
	uint32_t unshift(Block *blk){
		if (likely(_writable))return _ncli_ptr->unshift(_vtxno,blk);
		else throw_nynn_exception(0,"nyn_file is not writable");
	}
	uint32_t shift(Block *blk){
		if (likely(_writable))return _ncli_ptr->shift(_vtxno,blk);
		else throw_nynn_exception(0,"nyn_file is not writable");

	}
	uint32_t push(Block *blk){
		if (likely(_writable))return _ncli_ptr->push(_vtxno,blk);
		else throw_nynn_exception(0,"nyn_file is not writable");
	}
	uint32_t pop(Block *blk){
		if (likely(_writable))return _ncli_ptr->pop(_vtxno,blk);
		else throw_nynn_exception(0,"nyn_file is not writable");
	}
	Block* read_head(Block *blk){
		return this->read(nynn_file::headblkno,blk);
	}
	Block* read_tail(Block *blk){
		return this->read(nynn_file::tailblkno,blk);
	}
	Block* read(uint32_t blkno,Block *blk){
		if (likely(_local)){
			return _fs.get_sgs().read(_vtxno,blkno,blk);
		}else{
			return  _dcli_ptr->read(_vtxno,blkno,blk)?blk:NULL;
		}
	}
	bool vtx_exists(){
		if (likely(_local)){
			return _fs.get_sgs().vtx_exists(_vtxno);
		}else{
			return _dcli_ptr->vtx_exists(_vtxno);
		}
	}
private:
	nynn_fs& _fs;
	uint32_t _vtxno;
	bool _local;
	bool _writable;
	unique_ptr<nynn_ncli> _ncli_ptr;
	unique_ptr<nynn_dcli> _dcli_ptr;

	nynn_file(const nynn_file&);
	nynn_file& operator=(const nynn_file&);
};
}}
#endif
