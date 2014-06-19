#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;
int main(int argc,char**argv){
	string text_ip=argv[1];
	string text_port=argv[2];
	uint32_t vtxno=parse_int(argv[3],~0u);

	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	string serv_endpoint="tcp://"+text_ip+":"+text_port;
	sock.connect(serv_endpoint.c_str());
	log_i("connected to %s",serv_endpoint.c_str());
	prot::Requester req(sock);
	
	Block blk;
	CharContent *cctt=blk;
	while(pop(req,vtxno,0,&blk)!=INVALID_BLOCKNO){
		string s(cctt->begin(),cctt->end());
		cout<<s<<endl;
	}
	sock.close();
}

