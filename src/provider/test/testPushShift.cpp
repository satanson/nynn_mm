#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace nynn::mm::rpc;

int main(int argc,char**argv)
{
	string host=argv[1];
	uint32_t port=strtoul(argv[2],NULL,0);

	uint32_t vtxno=strtoul(argv[3],NULL,0);
	string srcFilePath=argv[4];
	string dstFilePath=argv[5];

	ProviderRPC prov(host,port);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);
	for (int i=0;i<keys.size();i++)prov.attachSubgraph(keys[i]);
	MmapFile srcMmapFile(srcFilePath);
	uint32_t fileLength=srcMmapFile.getLength();
	MmapFile dstMmapFile(dstFilePath,fileLength);
	char *srcBase=static_cast<char*>(srcMmapFile.getBaseAddress());
	char *dstBase=static_cast<char*>(dstMmapFile.getBaseAddress());

	log_i("src file=%d",srcMmapFile.getLength());
	log_i("dst file=%d",dstMmapFile.getLength());

	RawBlock rawblk;
	Block* blk=rawblk;//for presentation
	vector<int8_t> &xblk=rawblk;//for exchanging
	CharContent *content=*blk;

	//copy source file to subgraphset.
	prov.lock(vtxno,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t i=CharContent::CONTENT_CAPACITY;
	while(i<fileLength){
		content->resize(CharContent::CONTENT_CAPACITY);
		memcpy(content->begin(),
				srcBase+i-CharContent::CONTENT_CAPACITY,
				CharContent::CONTENT_CAPACITY);

		log_i("[%d,%d) %d Bytes",i-content->size(),i,content->size());
		prov.push(vtxno,xblk);
		i+=CharContent::CONTENT_CAPACITY;
	}
	if (i-fileLength>0){
		uint32_t restStart=i-CharContent::CONTENT_CAPACITY;
		uint32_t restSize=fileLength-restStart;

		content->resize(restSize);
		memcpy(content->begin(),
				srcBase+restStart,
				content->size());
				
		log_i("write[%d,%d) %d Bytes",restStart,restStart+restSize,content->size());
		prov.push(vtxno,xblk);
	}
	prov.unlock(vtxno);
	uint32_t tailBlkno=prov.getTailBlkno(vtxno);
	prov.read(vtxno,tailBlkno,xblk);
	string s;
	s.resize(content->size());
	std::copy(content->begin(),content->end(),s.begin());
	cout<<"----------------------------------------------"<<endl;
	cout<<s<<endl;
	cout<<"----------------------------------------------"<<endl;

	prov.lock(vtxno,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t blkno=prov.getHeadBlkno(vtxno);
	i=0;
	while(blkno!=INVALID_BLOCKNO){
		prov.read(vtxno,blkno,xblk);
		memcpy(dstBase+i,content->begin(),content->size());
		log_i("read[%d,%d) %d Bytes",i,i+content->size(),content->size());
		i+=content->size();
		prov.shift(vtxno);
		blkno=prov.getHeadBlkno(vtxno);
	}
	prov.unlock(vtxno);
}
