#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);
	uint32_t vtxno=strtoul(argv[3],NULL,0);
	

	ProviderRPC prov(host,port);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);
	for (int i=0;i<keys.size();i++)prov.attachSubgraph(keys[i]);

	RawBlock rawblk;
	Block* blk=rawblk;//for presentation
	vector<int8_t> &xblk=rawblk;//for exchanging
	CharContent *content=*blk;
	
	prov.lock(vtxno,IS_WRITABLE|IS_BLOCKING);

	string input;
	while(getline(cin,input)){
		if(input.size()>CharContent::CONTENT_CAPACITY)
			input.resize(CharContent::CONTENT_CAPACITY);
		content->resize(input.size());
		std::copy(input.begin(),input.end(),content->begin());
		prov.unshift(vtxno,xblk);
	}
	prov.unlock(vtxno);
}
