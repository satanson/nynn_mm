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
	nynn_file(nynn_fs& fs,uint32_t vtxno)
		:fs(fs),vtxno(vtxno)
	{
		uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
		if (unlikely(!fs.get_sgs()->exists(sgkey))){
			uint64_t hostport=fs.where(sgkey);
			here= (hostport>>32)==fs.get_localhost();
		}else{
			here=true;
		}
	}
	
	uint32_t unshift(Block *blk){
		return fs.get_ncli()->unshift(vtxno,blk);
	}
	uint32_t shift(Block *blk){
		return fs.get_ncli()->shift(vtxno,blk);
	}
	uint32_t push(Block *blk){
		return fs.get_ncli()->push(vtxno,blk); 
	}
	uint32_t pop(Block *blk){
		return fs.get_ncli()->pop(vtxno,blk);
	}
	Block* read(uint32_t blkno,Block *blk){
		if(here){
			return fs.get_sgs()->read(vtxno,blkno,blk);
		}else{
			return fs.get_dcli(vtxno)->read(vtxno,blkno,blk)?blk:NULL;
		}
	}
	bool vtx_exists(){
		if(here){
			return fs.get_sgs()->vtx_exists(vtxno);
		}else{
			return fs.get_dcli(vtxno)->vtx_exists(vtxno);
		}
	}
private:
	nynn_fs& fs;
	uint32_t vtxno;
	bool here;

	nynn_file(const nynn_file&);
	nynn_file& operator=(const nynn_file&);
};
}}
#endif
