#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{	
	if (argc<3){
		cout<<"getSubgraphKeys host port\n";
		exit(0);
	}
	string host=argv[1];
	int port=atoi(argv[2]);
	ProviderRPC prov(host,port);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);
	for(int i=0;i<keys.size();i++){
		cout<<keys[i]<<endl;
	}
}
