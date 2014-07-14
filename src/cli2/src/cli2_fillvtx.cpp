#include<nynn_fs.hpp>
#include<nynn_file.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;
typedef uint32_t (nynn_file::*Perform)(Block*);
typedef map<string,Perform> PerformMap;

int main(int argc,char**argv){

	string naddr=argv[1];
	string daddr=argv[2];
	uint32_t vtxno=parse_int(argv[3],~0u);
	string p=argv[4];

	PerformMap pmap;
	pmap["push"]=&nynn_file::push;
	pmap["unshift"]=&nynn_file::unshift;
	Perform perform=pmap[p];

	nynn_fs fs(naddr,daddr);
	nynn_file f(fs,vtxno);
	Block blk;
	CharContent *cctt=blk;
	string line;
	while(getline(cin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)line.resize(CharContent::CONTENT_CAPACITY);
		cctt->resize(line.size());
		std::copy(line.begin(),line.end(),cctt->begin());
		(f.*perform)(&blk);
	}
}
