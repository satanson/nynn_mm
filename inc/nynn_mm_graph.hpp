#ifndef NYNN_MM_GRAPH_HPP_BY_SATANSON
#define NYNN_MM_GRAPH_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_zmqprot.hpp>
#include<zmq.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;

namespace nynn{namespace mm{
class Graph{
public:
	Graph(){}
	void* handle_write(const void* options,void* wdata){
		WriteOptions& opts=*(WriteOptions*)options;
		uint32_t vtxno=opts.vtxno;
		uint32_t blkno=opts.blkno;
		uint32_t targetip=opts[opts.num()-1];

		if (unlikely(targetip!=m_ip)){
			throw_nynn_exception(0,"targetip!=m_ip");
		}

		uint32_t rc=(m_subgset.*writeops[opts.opcode])(vtxo,blkno,(Block*)wdata);
		if (rc==INVALID_BLOCKNO){
			throw_nynn_exception(0,"failed to write data to subgraphset");
		}
		
		if (opts.num()!=0){
			opts.shrink();
			uint32_t nextip=opts[opts.num()-1];
			if (m_datasocks.count(nextip)==0){
				string s=format("no link (%s->%s)",ip2host(m_ip).c_str(),ip2host(nextip).c_str());
				throw_nynn_exception(0,s.c_str());
			}
			prot::requester req(*m_datasocks[nextip].get());
			req.ask(*opts,wdata);
		}else{
			prot::replier(*m_namesock.get());
			prot::ans(true,NULL,0);
		}
	}
	void* handle_read(const void* options,void*rdata){
		ReadOptions& opts=*(ReadOptions*)options;
		uint32_t vtxno=opts.vtxno;
		uint32_t blkno=opts.blkno;
		uint32_t sgkey=SubgraphSet::VTXNO2SUBGRAPH(vtxno);
		//vertex not exist!
		if (unlikely(m_shards.count(sgkey)==0))return NULL;

		auto ptr2memfunc=&unordered_map<uint32_t,uint32_t>::operator[];
		uint32_t targetip=mfspinsync<uint32_t>(m_shardslock,m_shards,ptr2memfunc,sgkey);
		//vertex exists in local datanode
		if (likely(targetip==m_ip)){
			return m_subgset.read(vtxno,blkno,(Block*)rdata);
		//vertex exists in remote datanode
		}else{
			//look up data in cache
			if (likely(m_cache.read(vtxno,blkno,(Block*)rdata)!=NULL)){
				return rdata;
			}

			//fetch from remote target host
			prot::requester req(m_datasocks[targetip]);
			req.ask(prot::READ_CMD,opts,sizeof(ReadOptions),NULL,0);
			//succeed in parsing answering data from remote target host
			if(likely(req.parse_ans())){
				// succeed in fetch specified data from remote target host
				if (likely(req.get_status==prot::STATUS_OK)){
					memcpy(rdata,req.get_data(),sizeof(Block));
					m_cache.write(vtxno,blkno,(Block*)rdata);
					return rdata;
				// fail to fetch...
				}else{
					return NULL;
				}
			//fail to parse answering data.
			}else{
				return NULL;
			}
		}
	}
	void* submit(){
		prot::requester req(*m_namesock.get());
		vector<uint32_t> sgkeys=m_subgset.getAllSgkeys();
		SubmitOptions& opts=*SubmitOptions::make(m_ip,sgkeys.size());
		for (int i=0;i<opts.sgknum;i++){
			opts[i]=sgkeys[i];
		}
		req.ask(prot::CMD_SUBMIT,&opts,opts.size(),NULL,0);
		req.parse_ans();
		if (req.get_status()==STATUS_OK){
			auto ptr2memfunc=&Graph::merge_shards;
			mfspinsync<void>(m_shardslock,*this,ptr2memfunc,req.get_data());
		}
	}
	void* hello(){
		prot::requester req(*m_namesock.get());
		req.ask(prot::HELL0_CMD,NULL,0,NULL,0);
		req.parse_ans();
		if (req.get_status()==STATUS_OK){
			auto ptr2memfunc=&Graph::merge_shards;
			mfspinsync<void>(m_shardslock,*this,ptr2memfunc,req.get_data());
		}
	}

	Graph(const string& path,uint32_t nameip,uint16_t nameport,vector<uint32_t>& dataips,uint16_t dataport)
	:m_subgset(path),m_nameip(nameip):m_host(get_host()),m_ip(get_ip()){
		string name_endpoint="tcp://"+ip2string(nameip)+":"+to_string(nameport);
		zmq::context_t ctx;
		m_namesock.reset(new zmq::socket_t(ctx,ZMQ_REQ));
		m_namesock->connect(name_endpoint.c_str());
		for (int i=0;i<dataips.size();i++){
			string data_endpoint="tcp://"+ip2string(dataips[i])+":"+to_string(dataport);
			m_datasocks[i].reset(new zmq::socket_t(ctx,ZMQ_REQ));
			m_datasocks[i]->bind(data_endpoint.c_str());
		}
	}

private:
	void merge_shards(void* data){
		ShardTable& st=*(ShardTable*)data;
		for (int i=0;i<st.num();i++){
			m_shards[st[i].sgkey]=st[i].targetip;
		}
	}
	Graph(const Graph&);
	Graph& operator=(const Graph&);
	
	//SubgraphSet instance
	SubgraphSet m_subgset;
	//caching remote data
	GraphCache m_cache;
	//a table for mapping subgraphsets to hosts' ips
	unordered_map<uint32_t,uint32_t> m_shards;
	//mutual exclusively access m_shards;
	Monitor m_shardslock;
	//datanode's hostname of its own
	string m_host;
	//datanode's host's ip
	uint32_t m_ip;
	//host name of namenode
	uint32_t m_nameip;
	//connection to namenode
	unique_ptr<zmq::socket_t> m_namesock;
	//connection to other datanodes 
	unordered_map<uint32_t,unique_ptr<zmq::socket_t> > m_datasocks;
};
}}
#endif
