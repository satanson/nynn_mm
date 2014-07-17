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
	cout<<"size="<<vtxcache.size()<<endl;
	unordered_map<uint32_t,shared_ptr<Vertex> >::iterator it;
	for(it=vtxcache.begin();it!=vtxcache.end();it++){
		uint32_t vtxno=it->first;
		shared_ptr<Vertex> vtx=it->second;
		if (vtx->getExistBit()){
			cout<<"vtxno="<<vtxno<<endl;
			cout<<"\tsize:"<<vtx->size()<<endl;
			cout<<"\theadBlkno:"<<vtx->getHeadBlkno()<<endl;
			cout<<"\ttailBlkno:"<<vtx->getTailBlkno()<<endl;
			cout<<"---------------------"<<endl;
		}
	}
}
