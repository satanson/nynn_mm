#include<test.h>

int main(int argc,char**argv)
{
	string basedir=argv[1];
	uint32_t vtxno=strtoul(argv[2],NULL,0);
	string srcFilePath=argv[3];
	string dstFilePath=argv[4];

	SubgraphSet sgs(argv[1]);
	MmapFile srcMmapFile(srcFilePath);
	uint32_t fileLength=srcMmapFile.getLength();
	MmapFile dstMmapFile(dstFilePath,fileLength);
	char *srcBase=static_cast<char*>(srcMmapFile.getBaseAddress());
	char *dstBase=static_cast<char*>(dstMmapFile.getBaseAddress());

	log_i("src file=%d",srcMmapFile.getLength());
	log_i("dst file=%d",dstMmapFile.getLength());
	Block blk;
	CharContent *content=blk;
	//copy source file to subgraphset.
	sgs.lock(vtxno,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t i=CharContent::CONTENT_CAPACITY;
	while(i<fileLength){
		content->resize(CharContent::CONTENT_CAPACITY);
		memcpy(content->begin(),
				srcBase+i-CharContent::CONTENT_CAPACITY,
				CharContent::CONTENT_CAPACITY);

		log_i("[%d,%d) %d Bytes",i-content->size(),i,content->size());
		sgs.push(vtxno,&blk);
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
		sgs.push(vtxno,&blk);
	}
	sgs.unlock(vtxno);
	uint32_t tailBlkno=sgs.getTailBlkno(vtxno);
	sgs.read(vtxno,tailBlkno,&blk);
	string s;
	s.resize(content->size());
	std::copy(content->begin(),content->end(),s.begin());
	cout<<"----------------------------------------------"<<endl;
	cout<<s<<endl;
	cout<<"----------------------------------------------"<<endl;

	sgs.lock(vtxno,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t blkno=sgs.getTailBlkno(vtxno);
	i=0;
	while(blkno!=INVALID_BLOCKNO){
		sgs.read(vtxno,blkno,&blk);
		memcpy(dstBase+i,content->begin(),content->size());
		log_i("read[%d,%d) %d Bytes",i,i+content->size(),content->size());
		i+=content->size();
		sgs.pop(vtxno);
		blkno=sgs.getTailBlkno(vtxno);
	}
	sgs.unlock(vtxno);
}
