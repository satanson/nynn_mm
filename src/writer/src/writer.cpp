#include<nynn_mm_config.h>
#include<ProducerRPC.h>
using namespace nynn::mm::rpc;
int main(int argc, char**argv)
{
	string prodHost=argv[1];
	uint32_t prodPort=strtoul(argv[2],NULL,0);
	string file=argv[3]; 
	std::shared_ptr<ProducerRPC> prod(new ProducerRPC(prodHost,prodPort));


	ifstream fin(file);
	RawBlock rblk;
	vector<int8_t>& xblk=rblk;
	Block* blk=rblk;
	CharContent *content=*blk;
	uint32_t vtxno=0;
	char line[sizeof(CharContent)];
	while(fin.getline(line,sizeof(line))){
		content->resize(strlen(line));
		std::copy(line,line+strlen(line),content->begin());
		prod->push(vtxno,xblk);
		vtxno++;
	}
	cout<<"vtxno="<<vtxno<<endl;
	fin.close();
}

