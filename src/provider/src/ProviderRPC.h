#ifndef PROVIDER_RPC_H_BY_SATANSON
#define PROVIDER_RPC_H_BY_SATANSON
#include<nynn_mm_common.h>
#include "Provider.h"
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
using namespace nynn::mm::common;

namespace nynn{namespace mm {namespace rpc{
class ProviderRPC{
public:
	ProviderRPC(const string &host, int port)	
	{
		boost::shared_ptr<TTransport> socket(new TSocket(host,port));	
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
		m_client.reset(new ProviderClient(protocol));
		try{
			socket->open();
		}catch(TException &tx){
			throwNynnException("Fail to construct ProviderRPC object!");
		}
	}
	~ProviderRPC()
	{

	}
	bool createSubgraph(const int32_t subgraphKey) { return m_client->createSubgraph(subgraphKey); }

	bool destroySubgraph(const int32_t subgraphKey) { return m_client->destroySubgraph(subgraphKey); }

	bool attachSubgraph(const int32_t subgraphKey) { return m_client->attachSubgraph(subgraphKey); }

	bool detachSubgraph(const int32_t subgraphKey) { return m_client->detachSubgraph(subgraphKey); }

	void getSubgraphKeys(std::vector<int32_t> & keys) 
	{ 
		vector<int32_t> tempKeys;
		m_client->getSubgraphKeys(tempKeys);
		keys.resize(tempKeys.size());
		std::copy(tempKeys.begin(),tempKeys.end(),keys.begin());
	}

	bool lock(const int32_t vtxno, const int32_t flag) { return m_client->lock(vtxno, flag);}

	bool unlock(const int32_t vtxno) { return m_client->unlock(vtxno);}

	int32_t getSize(const int32_t vtxno) { return m_client->getSize(vtxno);}

	int32_t getHeadBlkno(const int32_t vtxno) { return m_client->getHeadBlkno(vtxno);}

	int32_t getTailBlkno(const int32_t vtxno) { return m_client->getTailBlkno(vtxno);}

	void readAllBlknos(uint32_t vtxno,std::vector<int32_t>& blknos)
	{
		m_client->readAllBlknos(blknos,vtxno);
	}

	void read(const int32_t vtxno,const int32_t blkno, std::vector<int8_t> & xblk) 
	{ 
		m_client->read(xblk, vtxno, blkno);
	}

	void readn(const int32_t vtxno,const int32_t blkno, const int32_t n,std::vector<int8_t>& xblks)
	{
		m_client->readn(xblks,vtxno,blkno,n);
	}

	int32_t insertPrev(const int32_t vtxno, const int32_t nextBlkno, const std::vector<int8_t> & xblk) 
	{ 
		return m_client->insertPrev(vtxno, nextBlkno, xblk);
	}

	int32_t insertNext(const int32_t vtxno, const int32_t prevBlkno, const std::vector<int8_t> & xblk) 
	{ 
		return m_client->insertNext(vtxno, prevBlkno, xblk);
	}

	bool remove(const int32_t vtxno, const int32_t blkno) { return m_client->remove(vtxno, blkno);}

	int32_t unshift(const int32_t vtxno, const std::vector<int8_t> & newHeadXBlk) 
	{ 
		return m_client->unshift(vtxno, newHeadXBlk);
	}

	bool shift(const int32_t vtxno) { return m_client->shift(vtxno);}

	int32_t push(const int32_t vtxno, const std::vector<int8_t> & newTailXBlk) {
	   	return m_client->push(vtxno, newTailXBlk);
	}
	bool pop(const int32_t vtxno) { return m_client->pop(vtxno);}
	
private:
	std::shared_ptr<ProviderClient> m_client;
};
}}}
#endif
