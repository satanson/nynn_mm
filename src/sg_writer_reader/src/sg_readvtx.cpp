#include<nynn_mm_config.hpp>
#include<sys/time.h>

typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vtxno=strtoul(argv[3],NULL,0);

	uint32_t firstBlkno=actid==string("shift")?HEAD_BLOCKNO:TAIL_BLOCKNO;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	Next next=nexts[actid];

	if (next==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	Block blk;
	CharContent *content=blk; 
	SubgraphSet sgs(basedir);
	vector<int32_t> sgkeys;
	sgs.getSubgraphKeys(sgkeys);
	for (int i=0;i<sgkeys.size();i++){
		sgs.attachSubgraph(sgkeys[i]);
	}
	uint32_t blkno=firstBlkno;
	while (blkno!=INVALID_BLOCKNO){
		sgs.read(vtxno,blkno,&blk);
		blkno=(blk.getHeader()->*next)();
		string  line(content->begin(),content->end());
		cout<<line<<endl;
	}
}
