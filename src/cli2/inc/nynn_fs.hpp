#ifndef NYNN_FS_HPP_BY_SATANSON
#define NYNN_FS_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_file.hpp>
using namespace std;
using namespace nynn::mm;
using namespace nynn::serv;
using namespace nynn::cli;
namespace nynn{namespace cli{
class nynn_fs{
	friend class nynn_file;
public:
	nynn_fs(uint32_t dsip,uint16_t dsport){

	}
	nynn_file get_file(uint32_t vtxno){
		return nynn_file(*this,vtxno);
	}
private:
	SubgraphSet _sgset;
	DataServiceClient _cli;
	nynn_fs(nynn_fs& const fs);
	nynn_fs& operator=(nynn_fs& const fs);
};
}}
#endif
