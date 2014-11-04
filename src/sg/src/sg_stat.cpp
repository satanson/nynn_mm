#include<nynn_mm_config.hpp>
#include<nynn_mm_edgemanip.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
int main(int argc,char**argv){
	string path=argv[1];

	MmapFile m(path);
	char* base=(char*)m.getBaseAddress();
	char* boundary=base+m.getLength();
	char* p=base;

	uint32_t minvtxno=UINT_MAX,maxvtxno=0u;
	int vtxcnt=0,edgecnt=0;
	while(p<boundary){
		++vtxcnt;
		uint32_t vtxno=*(uint32_t*)p;
		minvtxno=minvtxno>vtxno?vtxno:minvtxno;
		maxvtxno=maxvtxno<vtxno?vtxno:maxvtxno;

		p+=sizeof(uint32_t);
		uint32_t size=*(uint32_t*)p;
		edgecnt+=size;
		p+=sizeof(uint32_t);
		Edge* eb=(Edge*)p;
		p+=sizeof(Edge)*size;
	}
	cout<<"minvtxno="<<minvtxno<<endl;
	cout<<"maxvtxno="<<maxvtxno<<endl;
	cout<<"vtxcnt="<<vtxcnt<<endl;
	cout<<"edgecnt="<<edgecnt<<endl;
}

