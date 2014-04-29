#include<nynn_cli.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

int main(int argc,char**argv){

	string nhost=argv[1];
	uint32_t nport=parse_int(argv[2],~0);
	string dhost=argv[3];
	uint32_t dport=parse_int(argv[4],~0);

	uint32_t vtxno=parse_int(argv[5],~0u);
	nynn_cli cli(nhost,nport,dhost,dport);

	
	Block blk;
	CharContent *cctt=blk;
	string line;
	while(getline(cin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)line.resize(CharContent::CONTENT_CAPACITY);
		cctt->resize(line.size());
		std::copy(line.begin(),line.end(),cctt->begin());
		cli.push(vtxno,&blk);
	}
}
