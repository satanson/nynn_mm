#include<nynn_mm_config.h>
#include<sys/time.h>

typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string basedir=argv[2];
	uint32_t vtxno=strtoul(argv[3],NULL,0);

	map<string,First> firsts;
	firsts["pop"]=&SubgraphSet::getTailBlkno;
	firsts["shift"]=&SubgraphSet::getHeadBlkno;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	First first=firsts[actid];
	Next next=nexts[actid];

	if (first==NULL||next==NULL){
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
	sgs.lock(vtxno,false);
	uint32_t blkno=(sgs.*first)(vtxno);
	while (blkno!=INVALID_BLOCKNO){
		sgs.read(vtxno,blkno,&blk);
		blkno=(blk.getHeader()->*next)();
		string  line(content->begin(),content->end());
		cout<<line<<endl;
	}
	sgs.unlock(vtxno);
}
