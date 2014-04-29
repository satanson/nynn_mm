#ifndef NYNN_FILE_HPP_BY_SATANSON
#define NYNN_FILE_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_fs.hpp>
using namespace std;
using namespace nynn::mm;
using namespace nynn::serv;
using namespace nynn::cli;
namespace nynn{namespace cli{
class nynn_file{
	friend class nynn_fs;
private:
	nynn_fs& _fs;
	uint32_t _vtxno;
	nynn_file(nynn_fs&fs,uint32_t vtxno):_fs(fs),_vtxno(vtxno){}
	nynn_file(nynn_file& const rhs):_fs(rhs._fs),_vtxno(rhs._vtxno){}
	nynn_file& operator=(nynn_file& const rhs){this->_fs=rhs._fs;this->_vtxno=rhs._vtxno;return *this;}
};
}}
#endif
