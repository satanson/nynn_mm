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
<<<<<<< HEAD
	uint32_t vtxno=parse_int(argv[3],~0ul);
=======
	uint32_t vtxno=parse_int(argv[3],~0u);
>>>>>>> cef92da064ff2f8989a949b4ef422acc4d7d2310

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
		unshift(req,vtxno,0,&blk);
	}
	sock.close();
}

