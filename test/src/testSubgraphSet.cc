#include<test.h>

int main(int argc,char**argv)
{
	string basedir=argv[1];
	SubgraphSet sgs(argv[1]);
	sgs.createSubgraph(0);
	sgs.attachSubgraph(0);
	Block blk;
	CharContent *content=blk;
	string s;
	uint32_t vtxno=0;
	for(int i=0;i<SubgraphSet::VERTEX_INTERVAL_WIDTH;i++){
		getline(cin,s);
		content->resize(s.size());
		std::copy(s.begin(),s.end(),content->begin());
		sgs.push(vtxno,&blk);
	}

	uint32_t blkno=sgs.getTailBlkno(vtxno);
	while(blkno!=INVALID_BLOCKNO){
		sgs.read(vtxno,blkno,&blk);
		string s(content->begin(),content->end());
		cout<<s<<endl;
		blkno=blk.getHeader()->getPrev();
	}
}
