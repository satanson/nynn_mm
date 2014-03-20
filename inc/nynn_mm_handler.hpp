#ifndef NYNN_MM_HANDLER_HPP_BY_SATANSON
#define NYNN_MM_HANDLER_HPP_BY_SATANSON
#include<nynn_common.hpp>
#include<nynn_mm_graph.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_graph_table.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;



static auto writeops[OP_NUM]={
	&SubgraphSet::shift,
	&SubgraphSet::unshift,
	&SubgraphSet::push,
	&SubgraphSet::pop };

class Graph{
public:
	
	bool handle_write(const void* options,void* wdata){
		WriteOptions& oldopts=*(WriteOptions*)options;
		if (unlikely(sizeof(WriteOptions)!=oldopts.size())){
			throw_nynn_exception(0,"invalid WriteOptions");
		}
		uint32_t opcode=oldopts.opcode;
		uint32_t vtxno=oldopts.vtxno;
		uint32_t blkno=oldopts.blkno;

		uint32_t sgkey=SubgraphSet::VTXNO2SUBGRAPH(vtxno);
		// subgraphset non-exists yet!
		if (likely(m_partition[sgkey]==0)){
			auto ptr2mf=&GraphTable::create;
			mfspinsync<uint32_t>(m_monitor,*this,ptr2mf,sgkey);
		}
		// create a WriteOptions;
		IPSet& ips=getAllHostsOfSgkey(sgkey);
		WriteOptions& opts=*WriteOptions::make(opcode,vtxno,blkno,ips.size());
		unique_ptr<WriteOptions> fordelete(&opts);
		for (int i=0;i<ips.size();i++) opts[i]=ops[i];
		// request for writing operation
		prot::requester req(*m_datasocks[opts[opts.num()-1]].get());
		req.ask(CMD_WRITE,&opts,opts.size(),wdata,sizeof(Block));
		req.parse_ans();
		
		//check for if operation is successful
		uint8_t status=req.get_status();
		uint32_t lastip=*(uint32_t*)req.get_data();
		if (status==prot::STATUS_OK&& lastip=opts[0])return true;
		else return false;
	}
	void handle_hello(const void* options){
		HelloOptions& opts=*(HelloOptions*)options;
		vector<ShardPair> pairs=getShards();
		ShardTable& st=*ShardTable::make(pairs.size());
		unique_ptr<ShardTable> fordelete(&st);
		for(int i=0;i<st.num();i++){
			st[i].sgkey=pairs[i].first;
			st[i].targetip=pairs[i].second;
		}
		prot::replier req(*m_datasocks[opts.ip].get());
		req.ans(STATUS_OK,&st,st.size());
	}

	void handle_submit(const void*options){
		SubmitOptions& opts=*(SumbitOptions*)options;
		vector<uint32_t> sgkeys(opts.num(),0);
		for (int i=0;i<opts.num();i++)sgkeys[i]=opts[i];
		setallSgkeysOfHost(opts.ip,sgkeys);

		vector<ShardPair> pairs=getShards();
		ShardTable& st=*ShardTable::make(pairs.size());
		unique_ptr<ShardTable> fordelete(&st);
		for(int i=0;i<st.num();i++){
			st[i].sgkey=pairs[i].first;
			st[i].targetip=pairs[i].second;
		}
		prot::replier req(*m_datasocks[opts.ip].get());
		req.ans(STATUS_OK,&st,st.size());
	}
	void notify(zmq::socket_t& sock){
		prot::request req(sock);

		vector<ShardPair> pairs=getShards();
		ShardTable& st=*ShardTable::make(pairs.size());
		unique_ptr<ShardTable> fordelete(&st);
		for(int i=0;i<st.num();i++){
			st[i].sgkey=pairs[i].first;
			st[i].targetip=pairs[i].second;
		}
		req.ask(CMD_NOTIFY,&st,st.size(),NULL,0);
		req.parse_ans();
	}
#endif
