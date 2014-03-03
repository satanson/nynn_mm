#include<nynn_mm_config.h>
#include<ProviderRPC.h>
#include<sys/time.h>
using namespace nynn::mm::rpc;

typedef int32_t (ProviderRPC::*First)(int32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string actid=argv[1];
	string provHost=argv[2];
	int32_t provPort=strtoul(argv[3],NULL,0);
	uint32_t vtxnoBeg=strtoul(argv[4],NULL,0);
	uint32_t vtxnoEnd=strtoul(argv[5],NULL,0);

	map<string,First> firsts;
	firsts["pop"]=&ProviderRPC::getTailBlkno;
	firsts["shift"]=&ProviderRPC::getHeadBlkno;

	map<string,Next> nexts;
	nexts["pop"]=&Block::BlockHeader::getPrev;
	nexts["shift"]=&Block::BlockHeader::getNext;

	First first=firsts[actid];
	Next next=nexts[actid];

	if (first==NULL||next==NULL){
		cout<<"Unknown act '"<<actid<<"'"<<endl;
		exit(0);
	}

	ProviderRPC prov(provHost,provPort);
	vector<int32_t> keys;
	prov.getSubgraphKeys(keys);

	struct timeval beg_tv,end_tv;
	double t;

	gettimeofday(&beg_tv,NULL);
	for (int i=0;i<keys.size();i++)prov.attachSubgraph(keys[i]);
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"attach subgraph:["<<vtxnoBeg<<","<<vtxnoEnd<<")"<<endl;
	cout<<"time usage:"<<t<<endl;

	RawBlock rawblk;
	Block* blk=rawblk;//for presentation
	vector<int8_t> &xblk=rawblk;//for exchanging
	CharContent *content=*blk;

	gettimeofday(&beg_tv,NULL);
	for (uint32_t vtxno=vtxnoBeg;vtxno<vtxnoEnd;vtxno++) {

		uint32_t blkno=(prov.*first)(vtxno);
		while (blkno!=INVALID_BLOCKNO){
			prov.read(vtxno,blkno,xblk);
			blkno=(blk->getHeader()->*next)();
		}
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"write vtxno("<<vtxnoEnd-vtxnoBeg<<"): ["<<vtxnoEnd<<","<<vtxnoBeg<<")"<<endl;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"vtxno per second="<<(vtxnoEnd-vtxnoBeg)/t<<endl;
}
