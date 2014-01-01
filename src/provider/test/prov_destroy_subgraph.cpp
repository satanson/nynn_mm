#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);
	uint32_t sgkey=strtoul(argv[3],NULL,0);

	ProviderRPC prov(host,port);

	if(prov.detachSubgraph(sgkey)){
		cout<<"succeed in detaching subgraph:sgkey="<<sgkey<<endl;
	}else{
		cout<<"fail to detach subgraph:sgkey="<<sgkey<<endl;
	}

	if(prov.destroySubgraph(sgkey)){
		cout<<"succeed in destroying subgraph:sgkey="<<sgkey<<endl;
	}else{
		cout<<"fail to destory subgraph:sgkey="<<sgkey<<endl;
	}
}

