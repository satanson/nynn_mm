#ifndef AGENT_RPC_H_BY_SATANSON
#define AGENT_RPC_H_BY_SATANSON
#include "Agent.h"
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
class AgentRPC{
public:
	AgentRPC(const string &host, int port)
	{
		boost::shared_ptr<TTransport> socket(new TSocket(host,port));
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
		m_client.reset(new AgentClient(protocol));
		try{
			socket->open();
		}catch(TException &tx){
			throwNynnException("Fail to construct AgentRPC object!");
		}
	}

	~AgentRPC()
	{

	}
	bool lock(const int32_t vtxno) {
		return m_client->lock(vtxno);
	}

	bool unlock(const int32_t vtxno) {
		return m_client->unlock(vtxno);
	}

	int32_t getHeadBlkno(const int32_t vtxno) {
		return m_client->getHeadBlkno(vtxno);
	}

	int32_t getTailBlkno(const int32_t vtxno) {
		return m_client->getTailBlkno(vtxno);
	}

	void read(const int32_t vtxno, const int32_t blkno,std::vector<int8_t>& xblk){
		m_client->read(xblk,vtxno,blkno);
	}
private:
	std::shared_ptr<AgentClient> m_client;
};

}}}
#endif
