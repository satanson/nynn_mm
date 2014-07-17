#include<linuxcpp.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;

int main(int argc,char** argv){
	if (argc<2)exit(0);

	string  endpoint=string("tcp://")+argv[1];
	uint32_t vtxno=parse_int(argv[2],0);
	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	sock.connect(endpoint.c_str());
	cout<<"connected to "<<endpoint<<endl;

	prot::Requester req(sock);
	unordered_map<uint32_t,shared_ptr<Vertex> > vtxcache;
	vtx_batch(req,vtxno,vtxcache);
	shared_ptr<Vertex> vtx=vtxcache[vtxno];
	cout<<"vtxcache size="<<vtxcache.size()<<endl;
	cout<<"vtxno="<<vtx->getSource()<<endl;
	if (vtx->getExistBit()){
		unordered_map<uint64_t,shared_ptr<Block> > blkcache;
		uint32_t blkno=vtx->getHeadBlkno();
		blk_batch(req,vtxno,blkno,0,blkcache);

		shared_ptr<Block> blk=blkcache[vtxnoblkno(vtxno,blkno)];
		cout<<"blkcache size="<<blkcache.size()<<endl;
		cout<<"nbytes="<<blkcache.size()*sizeof(Block)<<endl;
		cout<<"blk.prev="<<blk->getHeader()->getPrev()<<endl;
		cout<<"blk.next="<<blk->getHeader()->getNext()<<endl;
		cout<<"blk.source="<<blk->getHeader()->getSource()<<endl;
		cout<<"blk.blkno="<<blk->getHeader()->getBlkno()<<endl;

		unordered_map<uint64_t,shared_ptr<Block>>::iterator it;
		for(it=blkcache.begin();it!=blkcache.end();it++){
			uint64_t vb=it->first;
			uint32_t blkno=vb&0xffffffff;
			uint32_t vtxno=vb>>32;

			shared_ptr<Block> blk=it->second;
			cout<<format("vtxno=%u blkno=%u",vtxno,blkno)<<endl;
		}
	}
}
