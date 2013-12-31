#ifndef PRODUCER_RPC_H_BY_SATANSON
#define PRODUCER_RPC_H_BY_SATANSON
#include "Producer.h"
#include<iostream>
#include<thrift/protocol/TBinaryProtocol.h>
#include<thrift/transport/TSocket.h>
#include<thrift/transport/TTransportUtils.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace boost;
using namespace nynn::mm;

namespace nynn{namespace mm {namespace rpc{
class ProducerRPC{
public:
	ProducerRPC(const string &host, int port)
	{
		boost::shared_ptr<TTransport> socket(new TSocket(host,port));
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
		m_client.reset(new ProducerClient(protocol));
		try{
			socket->open();
		}catch(TException &tx){
			throwNynnException("Fail to construct ProducerRPC object!");
		}
	}

	~ProducerRPC()
	{

	}

	bool report(const std::string& host, const std::vector<int32_t> & sgkeys) {
		return m_client->report(sgkeys,host);
	}
	void getHost(const int32_t sgkey,std::string& host) {
		m_client->getHost(host,sgkey);
	}

	int32_t insertPrev(const int32_t vtxno, const int32_t nextBlkno, const std::vector<int8_t> & xblk) { 
		return m_client->insertPrev(vtxno,nextBlkno,xblk);
	}

	int32_t insertNext(const int32_t vtxno, const int32_t prevBlkno, const std::vector<int8_t> & xblk) {
		return m_client->insertNext(vtxno,prevBlkno,xblk);
	}

	bool remove(const int32_t vtxno, const int32_t blkno) {
		return m_client->remove(vtxno,blkno);
	}

	int32_t unshift(const int32_t vtxno, const std::vector<int8_t> & newHeadXBlk) {
		return m_client->unshift(vtxno,newHeadXBlk);
	}

	bool shift(const int32_t vtxno) {
		return m_client->shift(vtxno);
	}

	int32_t push(const int32_t vtxno, const std::vector<int8_t> & newTailXBlk) {
		return m_client->push(vtxno,newTailXBlk);
	}

	bool pop(const int32_t vtxno) {
		return m_client->pop(vtxno);
	}
private:

	std::shared_ptr<ProducerClient> m_client;

};
}}}
#endif
