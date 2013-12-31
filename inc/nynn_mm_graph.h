#include<nynn_mm_config.h>
#include<ProviderRPC.h>
#include<ProducerRPC.h>
using namespace std;
using namespace nynn::mm::common;
using namespace nynn::mm;
using namespace nynn::mm::rpc;

namespace nynn{namespace mm{

class Graph{
public:
	typedef map<string,std::shared_ptr<ProviderRPC> > Host2PrividerMap;
	typedef map<uint32_t,string> Subgraph2HostMap;
	typedef Host2PrividerMap::iterator Host2PrividerMapIterator;
	typedef Subgraph2HostMap::iterator Subgraph2HostMapIterator;

	Graph(string prodHost,uint32_t prodPort,string selfHost,uint32_t port,vector<string> hosts)
		:m_prodHost(prodHost),m_prodPort(prodPort),m_localHost(selfHost),m_provPort(port)
	{
		try{
			//construct ProducerRPC object.
			m_producer.reset(new ProducerRPC(m_prodHost,m_prodPort));
			//construct ProviderRPC objects.
			for (int i=0;i<hosts.size();i++){
				string &host=hosts[i];
				m_host2Provider[host].reset(new ProviderRPC(host,m_provPort));
			}
			vector<int32_t> sgkeys;
			m_host2Provider[m_localHost]->getSubgraphKeys(sgkeys);
			m_producer->report(m_localHost,sgkeys);
		}catch(NynnException &ex){
			throwNynnException("Failed to complete construction of Graph Object");
		}
	}

	
	
	bool lock(uint32_t vtxno,bool nonblocking)
	{
		string host=getHostTS(vtxno);
		uint32_t flag=(nonblocking?IS_NONBLOCKING:IS_BLOCKING)|IS_READABLE;

		if (host!="UNKNOWN") return getProvider(host)->lock(vtxno,flag);

		refresh(vtxno);
		
		host=getHostTS(vtxno);
		if (host!="UNKNOWN"){
			return getProvider(host)->lock(vtxno,flag);
		}else{
			return false;
		}
	}

	bool unlock(uint32_t vtxno)
	{
		string host=getHostTS(vtxno);
		if (host!="") return getProvider(host)->unlock(vtxno);
		return false;
	}

	uint32_t getHeadBlkno(uint32_t vtxno) { return getProvider(vtxno)->getHeadBlkno(vtxno); }
	uint32_t getTailBlkno(uint32_t vtxno) { return getProvider(vtxno)->getTailBlkno(vtxno); }

	bool read(uint32_t vtxno,uint32_t blkno,vector<int8_t>& xblk)
	{
		string host=getHost(vtxno);
		Block *blk=reinterpret_cast<Block*>(xblk.data());

		if (isNative(host)){
			getProvider(host)->read(vtxno,blkno,xblk);
		}else if (isAlien(host)){
			if (!m_graphCache.read(vtxno,blkno,blk)){
				vector<int8_t> xblks;
				getProvider(host)->readn(vtxno,blkno,4096/sizeof(Block),xblks);
				Block* blk=reinterpret_cast<Block*>(xblks.data());
				int blkNum=xblks.size()/sizeof(Block);
				int i=0;
				std::copy(&xblks[0],&xblks[0]+sizeof(Block),xblk.begin());
				do{
					m_graphCache.write(vtxno,blkno,blk);
					blkno=blk->getHeader()->getNext();
					blk+=1;
					i++;
				}while(i<blkNum);
			}
		}else{
			log_w("Oops! Never reach here!");
		}
	}

private:
	//thread safe.
	string getHostTS(uint32_t vtxno){
		SharedSynchronization ss(&m_subgraph2HostRWLock);
		uint32_t subgraphKey=SubgraphSet::VTXNO2SUBGRAPH(vtxno);
		if (m_subgraph2Host.find(subgraphKey)!=m_subgraph2Host.end()){
			return m_subgraph2Host[subgraphKey];
		}
		return "UNKNOWN";
	}
	string getHost(uint32_t vtxno){
		return m_subgraph2Host[SubgraphSet::VTXNO2SUBGRAPH(vtxno)];
	}

	void refresh(uint32_t vtxno)
	{
		ExclusiveSynchronization  es(&m_subgraph2HostRWLock);
		string host;
		uint32_t sgkey=SubgraphSet::VTXNO2SUBGRAPH(vtxno);
		m_producer->getHost(sgkey,host);
		m_subgraph2Host[sgkey]=host;
	}

	bool isNative(const string& host) { return m_localHost==host; }
	bool isAlien(const string& host) { return m_localHost!=host; }

	std::shared_ptr<ProviderRPC>& getProvider(const string& host)
	{
		return m_host2Provider[host];
	}

	std::shared_ptr<ProviderRPC>& getProvider(uint32_t vtxno){
		return m_host2Provider[m_subgraph2Host[SubgraphSet::VTXNO2SUBGRAPH(vtxno)]];
	}

	string 	   m_prodHost;
	uint32_t   m_prodPort;
	std::shared_ptr<ProducerRPC> m_producer;
	string     m_localHost;
	uint32_t 		 m_provPort;
	Host2PrividerMap m_host2Provider;
	Subgraph2HostMap m_subgraph2Host;
	GraphCache       m_graphCache;
	RWLock 			 m_subgraph2HostRWLock;
};
}}
