#include<nynn_mm_config.hpp>
#include<nynn_common.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
int main(int argc,char **argv)
{
	try{
		string basedir=argv[1];
		uint32_t sgkey=parse_int(argv[2],0);
		SubgraphSet sgs(basedir);
		sgkey=SubgraphSet::VTXNO2SGKEY(sgkey);
		if (unlikely(sgs.vtx_exists(sgkey)))sgs.destroySubgraph(sgkey);
		sgs.createSubgraph(sgkey);
	}catch(nynn_exception_t& err){
		cout<<err.what()<<endl;
	}catch(...){
		cout<<"An exception occurs!"<<endl;
	}

}
