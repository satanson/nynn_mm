#include<nynn_mm_config.hpp>
#include<nynn_mm_edgemanip.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
typedef edge_manip_t<Edge> EdgeManip;
typedef void (EdgeManip::*InsertFunc)(uint32_t,Edge*begin,Edge*end);
int main(int argc,char**argv){
	string basedir=argv[1];
	string path=argv[2];
	string act=argv[3];
	InsertFunc insert;
	if(act=="push")insert=&EdgeManip::push_edges;
	else if(act=="unshift")insert=&EdgeManip::unshift_edges;
	else exit(0);

	SubgraphSet sgs(basedir);
	EdgeManip edgemanip(sgs);

	MmapFile m(path);
	char* base=(char*)m.getBaseAddress();
	char* boundary=base+m.getLength();
	char* p=base;

	size_t edgenum=0;
	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	
	while(p<boundary){
		uint32_t vtxno=*(uint32_t*)p;
		p+=sizeof(uint32_t);
		uint32_t size=*(uint32_t*)p;
		p+=sizeof(uint32_t);
		Edge* eb=(Edge*)p;
		p+=sizeof(Edge)*size;
		(edgemanip.*insert)(vtxno,eb,eb+size);
		edgenum+=size;
	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	cout<<"timeusage:"<<t<<endl;
	cout<<"edgenum:"<<edgenum<<endl;
	cout<<"edgenum_per_s:"<<edgenum/(t+0.0)<<endl;
	cout<<"nbytes:"<<edgenum*sizeof(Edge)<<endl;
	cout<<"throughput:"<<edgenum*sizeof(Edge)/(t+0.0)<<endl;
}
