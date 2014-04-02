#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>
typedef uint32_t (SubgraphSet::*First)(uint32_t);
typedef uint32_t (Block::BlockHeader::*Next)();

int main(int argc,char**argv)
{
	string text_ip=argv[1];
	string text_port=argv[2];
	uint32_t vtxno_begin=parse_int(argv[3],~0ul);
	uint32_t vtxno_end=parse_int(argv[4],~0ul);
	string actid=argv[5];
	assert(vtxno_begin!=~0ul);
	assert(vtxno_end!=~0ul);
	assert(actid=="pop"||actid=="shift");

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
	uint32_t blkno = actid=="pop"?
					 TAIL_BLOCKNO
					 :
					 HEAD_BLOCKNO;

	struct timeval beg_tv,end_tv;
	double t;
	
	Block blk;
	CharContent *cctt=blk;
	uint32_t firstblkno = actid=="pop"?
					 TAIL_BLOCKNO
					 :
					 HEAD_BLOCKNO;

	gettimeofday(&beg_tv,NULL);
	for (uint32_t vtxno=vtxno_begin;vtxno<vtxno_end;vtxno++) {
		uint32_t blkno=firstblkno;
		while(blkno!=INVALID_BLOCKNO){
			if(!read(req,vtxno,blkno,&blk))break;
			blkno=(blk.getHeader()->*next)();
		}
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"write vtxno("<<vtxno_end-vtxno_begin<<"): ["<<vtxno_begin<<","<<vtxno_end<<")"<<endl;
	cout<<"time usage:"<<t<<"s"<<endl;
	cout<<"vtxno per second="<<(vtxno_end-vtxno_begin)/t<<endl;
}
