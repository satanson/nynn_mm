#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;
typedef uint32_t (*Act)(prot::Requester&,uint32_t,uint32_t,Block*);
int main(int argc,char**argv){
	string text_ip=argv[1];
	string text_port=argv[2];
	uint32_t vtxno=parse_int(argv[3],~0ul);
	string actid=argv[4];
	map<string,Act> actMap;
	actMap["unshift"]=unshift;
	actMap["push"]=push;
	assert(actMap.count(actid)>0);
	Act act=actMap[actid];

	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	string serv_endpoint="tcp://"+text_ip+":"+text_port;
	sock.connect(serv_endpoint.c_str());
	log_i("connected to %s",serv_endpoint.c_str());
	prot::Requester req(sock);
	
	Block blk;
	CharContent *cctt=blk;
	string line;
	while(getline(cin,line)){
		if (line.size()>CharContent::CONTENT_CAPACITY)line.resize(CharContent::CONTENT_CAPACITY);
		cctt->resize(line.size());
		std::copy(line.begin(),line.end(),cctt->begin());
		(*act)(req,vtxno,0,&blk);
	}
	sock.close();
}
