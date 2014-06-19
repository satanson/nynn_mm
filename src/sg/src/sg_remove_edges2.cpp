#include<nynn_mm_config.hpp>
#include<nynn_mm_edgemanip.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
typedef edge_manip_t<Edge> EdgeManip;
typedef size_t (EdgeManip::*RemoveFunc)(uint32_t,size_t);
int main(int argc,char**argv){
	string basedir=argv[1];
	uint32_t vbegin=parse_int(argv[2],~0);
	uint32_t vend=parse_int(argv[3],~0);
	uint32_t unit=parse_int(argv[4],1);
	string act=argv[5];

	RemoveFunc remove;
	if(act=="pop")remove=&EdgeManip::pop_edges;
	else if(act=="shift")remove=&EdgeManip::shift_edges;
	else exit(0);

	SubgraphSet sgs(basedir);
	EdgeManip edgemanip(sgs);

	size_t edgenum=0;
	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for(uint32_t vtxno=vbegin;vtxno<vend;vtxno++){
		uint32_t k=0;
		while((k=(edgemanip.*remove)(vtxno,unit))==unit)edgenum+=unit;
		edgenum+=k;
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
