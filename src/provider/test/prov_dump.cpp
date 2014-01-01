#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);

	ProviderRPC prov(host,port);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);
	cout<<keys.size()<<" subgraphs"<<endl;
	for(int i=0;i<keys.size();i++){
		cout<<"subgraph["<<keys[i]<<","<<keys[i]+SubgraphSet::VERTEX_INTERVAL_WIDTH<<")"<<endl;
	}
}
