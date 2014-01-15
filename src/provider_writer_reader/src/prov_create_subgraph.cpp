#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);
	uint32_t sgkey=strtoul(argv[3],NULL,0);

	ProviderRPC prov(host,port);
	if(prov.createSubgraph(sgkey)){
		cout<<"succeed in creating subgraph:sgkey="<<sgkey<<endl;
	}else{
		cout<<"fail to create subgraph:sgkey="<<sgkey<<endl;
	}
	if(prov.attachSubgraph(sgkey)){
		cout<<"succeed in attaching subgraph:sgkey="<<sgkey<<endl;
	}else{
		cout<<"fail to attach subgraph:sgkey="<<sgkey<<endl;
	}
}
