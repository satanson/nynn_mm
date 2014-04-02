#ifndef NYNN_MM_HANDLER_HPP_BY_SATANSON
#define NYNN_MM_HANDLER_HPP_BY_SATANSON
#include<zmq.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_graph.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_graph_table.hpp>

using namespace std;
using namespace nynn;
using namespace nynn::mm;
namespace nynn{namespace mm{

enum{WRITE_UNSHIFT,WRITE_SHIFT,WRITE_PUSH,WRITE_POP,WRITE_OP_NUM};

typedef unordered_map<uint32_t,shared_ptr<zmq::socket_t> > ZMQSockMap;
typedef ZMQSockMap::iterator ZMQSockMapIter;

void submit(prot::Requester& req,Graph& g,RWLock& glk){
	//SubmitOptions& sbmtopts=*SubmitOptions::make(g.get_sgkey_num());
	//unique_ptr<void> just_for_auto_delete(&sbmtopts);
	//g.get_sgkeys(sbmtopts.begin(),sbmtopts.end());
	auto ptr2mf=&Graph::create_submit_options;
	SubmitOptions& sbmtopts=*mfsyncr<SubmitOptions*>(glk,g,ptr2mf);
	unique_ptr<void> just_for_auto_delete(&sbmtopts);
	sbmtopts->ip=get_ip();

	req.ask(prot::CMD_SUBMIT,&sbmtopts,sbmtopts.size(),NULL,0);
	req.parse_ans();
	uint32_t status=req.get_status();
	if (status==prot::STATUS_OK){
		log_i("succeed in submitting local sgkeys");
	}else{
		throw_nynn_exception(0,"failed to sumbit local sgkeys");
	}
}

void handle_submit(prot::Replier& rep,GraphTable& gt,Monitor& gtlk){
	SubmitOptions& sbmtopts=*(SubmitOptions*)rep.get_options();
	//gt.submit_sgkeys(sbmtopts->ip,sbmtopts.begin(),sbmtopts.end());
	auto ptr2mf=&GraphTable::submit_sgkeys;
	mfspinsync<void>(gtlk,gt,ptr2mf,sbmtopts->ip,sbmtopts.begin(),sbmtopts.end());
	rep.ans(prot::STATUS_OK,NULL,0);
}
void hello(prot::Requester& req,Graph& g,RWLock& glk){
	HelloOptions& hlopts=*HelloOptions::make(0);
	unique_ptr<void> just_for_auto_delete(&hlopts);
	req.ask(prot::CMD_HELLO,&hlopts,hlopts.size(),NULL,0);
	req.parse_ans();
	if (req.get_status()!=prot::STATUS_OK){
		throw_nynn_exception(0,"failed to get answer of prot::CMD_HELLO from namenode");
	}
	//g.merge_shard_table((ShardTable*)req.get_data());
	auto ptr2mf=&Graph::merge_shard_table;
	mfsyncw<void>(glk,g,ptr2mf,req.get_data());
	
}
void notify(ZMQSockMap& datasocks){
	ZMQSockMapIter it=datasocks.begin();
	NotifyOptions& ntfopts=*NotifyOptions::make(0);
	unique_ptr<void> just_for_auto_delete(&ntfopts);
	while(it!=datasocks.end()){
		prot::Requester req(*it->second.get());
		req.ask(prot::CMD_NOTIFY,&ntfopts,ntfopts.size(),NULL,0);
		req.parse_ans();
		it++;
	}
}
void handle_notify(prot::Replier& rep,pthread_t id){
	rep.ans(prot::STATUS_OK,NULL,0);
	pthread_kill(id,SIGHUP);
}
void handle_hello(prot::Replier &rep,GraphTable& gt,Monitor& gtlk){
	//ShardTable& st=*ShardTable::make(gt.get_shard_table_size());
	//unique_ptr<void> just_for_auto_delete(&st);
	//gt.get_shard_table(st.begin(),st.end());
	auto ptr2mf=&GraphTable::create_shard_table;
	ShardTable& st=*mfspinsync<ShardTable*>(gtlk,gt,ptr2mf);
	unique_ptr<void> just_for_auto_delete(&st);

	rep.ans(prot::STATUS_OK,&st,st.size());
}
uint32_t write(prot::Requester& req,uint32_t op,uint32_t vtxno,uint32_t blkno,Block* blk){
	WriteOptions& wrtopts=*WriteOptions::make(0);
	wrtopts->op=op;
	wrtopts->vtxno=vtxno;
	wrtopts->blkno=blkno;
	req.ask(prot::CMD_WRITE,&wrtopts,wrtopts.size(),blk,sizeof(Block));
	req.parse_ans();
	if (req.get_status()!=prot::STATUS_OK){
		string errinfo=string()+"failet to write into system:"
					  +"op="+to_string(op)+","
					  +"vtxno="+to_string(vtxno)+","
					  +"blkno="+to_string(blkno);
		throw_nynn_exception(0,errinfo.c_str());
	}
	return *(uint32_t*)req.get_data();
}

uint32_t unshift(prot::Requester& req,uint32_t vtxno,uint32_t blkno,Block*blk){
	return write(req,WRITE_UNSHIFT,vtxno,blkno,blk);
}
uint32_t shift(prot::Requester& req,uint32_t vtxno,uint32_t blkno,Block*blk){
	return write(req,WRITE_SHIFT,vtxno,blkno,blk);
}
uint32_t push(prot::Requester& req,uint32_t vtxno,uint32_t blkno,Block*blk){
	return write(req,WRITE_PUSH,vtxno,blkno,blk);
}
uint32_t pop(prot::Requester& req,uint32_t vtxno,uint32_t blkno,Block*blk){
	return write(req,WRITE_POP,vtxno,blkno,blk);
}
void handle_write_gt(prot::Replier& rep,GraphTable& gt,Monitor& gtlk,ZMQSockMap& datasocks){
	WriteOptions& old_wrtopts=*(WriteOptions*)rep.get_options();

	//uint32_t vtxno=old_wrtopts->vtxno;
	//uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(vtxno);
	//bool already_exists=gt.exists(sgkey);
	//if (!already_exists)gt.create(sgkey);

	//uint32_t replicas_num=gt.get_replicas_num(sgkey);
	//WriteOptions& wrtopts=*WriteOptions::make(replicas_num);
	//unique_ptr<void> just_for_auto_delete(&wrtopts);
	//memcpy(&wrtopts,&old_wrtopts,sizeof(WriteOptions::FixedType));
	//gt.get_replicas_hosts(sgkey,wrtopts.begin(),wrtopts.end());

	int already_exists=0;
	int vtxno=old_wrtopts->vtxno;
	auto ptr2mf=&GraphTable::createWriteOptions;
	WriteOptions& wrtopts=*mfspinsync<WriteOptions*>(gtlk,gt,ptr2mf,vtxno,already_exists);
	unique_ptr<void> just_for_auto_delete(&wrtopts);
	wrtopts->op=old_wrtopts->op;
	wrtopts->vtxno=old_wrtopts->vtxno;
	wrtopts->blkno=old_wrtopts->blkno;

	prot::Requester req(*datasocks[wrtopts[-1]]);
	req.ask(prot::CMD_WRITE,&wrtopts,wrtopts.size(),rep.get_data(),sizeof(Block));
	req.parse_ans();
	rep.ans(req.get_status(),req.get_data(),req.get_data_size());
	if (!already_exists)notify(datasocks);
}
void handle_write_g(prot::Replier& rep,Graph& g,RWLock& glk,ZMQSockMap& datasocks){
	WriteOptions& wrtopts=*(WriteOptions*)rep.get_options();
	//uint32_t sgkey=SubgraphSet::VTXNO2SGKEY(wrtopts->vtxno);
	//if (!g.exists(sgkey)){
	//	g.create(sgkey);
	//}
	//uint32_t rc=(g.*write_ops[wrtopts->op])(wrtopts->vtxno,wrtopts->blkno,rep.get_data());
	int op=wrtopts->op;
	uint32_t vtxno=wrtopts->vtxno;
	uint32_t blkno=wrtopts->blkno;
	void *data=rep.get_data();

	auto ptr2mf=&Graph::write;
	uint32_t rc=mfsyncw<uint32_t>(glk,g,ptr2mf,op,vtxno,blkno,data);

	wrtopts.shrink(1);
	//write to next datanode,if it's not last write operation.
	if (wrtopts){
		uint32_t nextip=wrtopts[-1];
		log_i("nextip=%s",ip2string(nextip).c_str());
		log_i("datasocks.count(%d)=%d",nextip,datasocks.count(nextip));
		prot::Requester req(*datasocks[wrtopts[-1]].get());
		//pipeline writing
		req.ask(prot::CMD_WRITE,&wrtopts,wrtopts.size(),data,sizeof(Block));
		req.parse_ans();
		if (unlikely(req.get_status()!=prot::STATUS_OK)){
			throw_nynn_exception(0,"can't write to next datanode");
		}
		assert(rc==*(uint32_t*)req.get_data());
	}
	rep.ans(prot::STATUS_OK,&rc,sizeof(rc));
}
void *read(prot::Requester& req,uint32_t vtxno,uint32_t blkno,Block* blk){
	ReadOptions& rdopts=*ReadOptions::make(0);
	unique_ptr<void> just_for_auto_delete(&rdopts);
	rdopts->vtxno=vtxno;
	rdopts->blkno=blkno;
	req.ask(prot::CMD_READ,&rdopts,rdopts.size(),NULL,0);
	req.parse_ans();
	if (likely(req.get_status()==prot::STATUS_OK)){
		memcpy(blk,req.get_data(),sizeof(Block));
		return blk;
	}else{
		return NULL;
	}
}
void handle_read(prot::Replier& rep,Graph& g,RWLock& glk,uint32_t localip,ZMQSockMap& datasocks){
	ReadOptions& rdopts=*(ReadOptions*)rep.get_options();
	//uint32_t targetip=g.which_host(rdopts->vtxno);
	//Block blk;
	////vtx non-exists!
	//if(targetip==0){
	//	rep.ans(prot::STATUS_ERR,NULL,0);
	//	return;
	//	//vtx exists in local host
	//}else if (targetip==localip){
	//	void *retblk=g.read(rdopts->vtxno,rdopts->blkno,&blk);
	//	if (unlikely(retblk==NULL)){
	//		rep.ans(prot::STATUS_ERR,NULL,0);
	//	}else{
	//		rep.ans(prot::STATUS_OK,&blk,sizeof(Block));
	//	}
	//	return;
	//	//vtx exists in remote host
	//}
	uint32_t vtxno=rdopts->vtxno;
	uint32_t blkno=rdopts->blkno;
	Block blk;
	uint32_t targetip=0;

	auto ptr2mf=&Graph::read;
	void *retblk=mfsyncr<void*>(glk,g,ptr2mf,vtxno,blkno,&blk,targetip,localip);

	if (likely(targetip==localip)){
		if (likely(retblk!=NULL)){
			rep.ans(prot::STATUS_OK,&blk,sizeof(Block));
			return;
		}else{
			rep.ans(prot::STATUS_ERR,NULL,0);
			return;
		}
	}else if (targetip==0){
		rep.ans(prot::STATUS_ERR,NULL,0);
		return;
	}
	//vtx has been pre-cached in local cache,look up cache first
	if (g.read_cache(vtxno,blkno,&blk)){
		rep.ans(prot::STATUS_OK,&blk,sizeof(Block));
		return;
	//vtx non-exists in local cache,request data from remote host
	}else{
		prot::Requester req(*datasocks[targetip].get());
		req.ask(prot::CMD_READ,&rdopts,rdopts.size(),NULL,0);
		req.parse_ans();
		//successfully
		if (likely(req.get_status()==prot::STATUS_OK)){
			g.write_cache(rdopts->vtxno,rdopts->blkno,&blk);
			rep.ans(prot::STATUS_OK,req.get_data(),sizeof(Block));
			return;
		//failed to fetch data from remote host
		}else{
			rep.ans(prot::STATUS_ERR,NULL,0);
			return;
		}
	}
}
}}
#endif
