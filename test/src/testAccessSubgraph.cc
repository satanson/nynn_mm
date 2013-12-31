#include<nynn_mm_subgraph_storage.h>
#include<test.h>
using namespace nynn::mm;

int main(int argc,char**argv)
{
	string path(argv[1]);
	try{
		Subgraph s(path);
		for(uint32_t i=0;i<1<<17;i++){
			cout<<"loop i="<<i<<endl;
			cout<<"block#"<<s.require()<<endl;
		}
		for(uint32_t i=0;i<1<<17;i++)s.release((1<<17)-1-i);
	}catch(NynnException &err){
		err.printBacktrace();
	}	
}
