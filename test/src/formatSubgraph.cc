#include<nynn_mm_subgraph_storage.h>
#include<test.h>
using namespace nynn::mm;

int main(int argc,char**argv)
{
	string path(argv[1]);
    try{
		Subgraph::format(path);
	}catch(NynnException &err){
		err.printBacktrace();
	}	
}
