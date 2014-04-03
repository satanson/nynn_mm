#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;
typedef uint32_t (Block::BlockHeader::*Next)();
int main(int argc,char**argv){
	string text_ip=argv[1];
	string text_port=argv[2];
	uint32_t vtxno=parse_int(argv[3],~0u);
	string actid=argv[4];
	assert(vtxno!=~0u);
	assert(actid=="pop"||actid=="shift");
	log_i("vtxno=%u",vtxno);
	Next next = actid=="pop"?
				&Block::BlockHeader::getPrev
				:
				&Block::BlockHeader::getNext;

	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	string serv_endpoint="tcp://"+text_ip+":"+text_port;
	sock.connect(serv_endpoint.c_str());
	log_i("connected to %s",serv_endpoint.c_str());
	prot::Requester req(sock);
	Block blk;
	CharContent *cctt=blk;
	uint32_t blkno = actid=="pop"?
					 TAIL_BLOCKNO
					 :
					 HEAD_BLOCKNO;

	while(blkno!=INVALID_BLOCKNO){
		if(!read(req,vtxno,blkno,&blk))break;
		blkno=(blk.getHeader()->*next)();
		string line(cctt->begin(),cctt->end());
		cout<<line<<endl;
	}
	sock.close();
}
