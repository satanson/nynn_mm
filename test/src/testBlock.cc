#include<nynn_mm_types.h>
using namespace nynn::mm;
typedef BlockType<512> Block512B;
typedef BlockType<1024>Block1KB;
typedef BlockType<4096>Block4KB;
int main(){
	Block512B blk0;
	cout<<"Block512B: "<<sizeof(Block512B)<<endl;
	cout<<"Block512B::BlockHeader: "<<sizeof(Block512B::BlockHeader)<<endl;
	cout<<"Block512B::CharContent: "<<sizeof(Block512B::CharContent)<<endl;
	cout<<"Block1KB:"<<sizeof(Block1KB)<<endl;
	cout<<"Block1KB::BlockHeader: "<<sizeof(Block1KB::BlockHeader)<<endl;
	cout<<"Block1KB::CharContent: "<<sizeof(Block1KB::CharContent)<<endl;
	cout<<"Block4KB: "<<sizeof(Block4KB)<<endl;
	cout<<"Block4KB::BlockHeader: "<<sizeof(Block4KB::BlockHeader)<<endl;
	cout<<"Block4KB::CharContent: "<<sizeof(Block4KB::CharContent)<<endl;
	
	Block512B::BlockHeader *header=blk0.getHeader();
	header->setSource(300);
	header->setPrev(1);
	header->setNext(3);
	header->setInfTimestamp(str2time("1987-06-15 12:30:00"));
	header->setSupTimestamp(time(NULL));
	cout<<*header<<endl;

	Block512B::CharContent *content=blk0;
	string s="ny name is ranpanf";
	content->resize(s.size());
	std::copy(s.begin(),s.end(),content->begin());
	string s0;
	s0.resize(content->size());
	std::copy(content->begin(),content->end(),s0.begin());
	cout<<s0<<endl;

	Block512B::TContent<double >*doubleContent=blk0;
	cout<<"sizeof(doubleContent)="<<sizeof(*doubleContent)<<endl;

	typedef Block512B::TContent<Edge> EdgeContent;
	cout<<"sizeof(EdgeContent)="<<sizeof(EdgeContent)<<endl;
	cout<<"EdgeContent:CONTENT_CAPACITY="<<EdgeContent::CONTENT_CAPACITY<<endl;
}
