#include<nynn_mm_config.hpp>
int main(int argc,char** argv){
	string basedir=argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	SubgraphSet sgs(basedir);
	while(sgs.pop(vtxno,NULL)!=INVALID_BLOCKNO);
}
