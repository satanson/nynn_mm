#ifndef NYNN_MM_GRAPH_TABLE_BY_SATANSON
#define NYNN_MM_GRAPH_TABLE_BY_SATANSON
#include<nynn_common.hpp>
using namespace std;
using namespace nynn;
namespace nynn{namespace mm{
class GraphTable{
public:
	typedef set<uint32_t> IPSet;
	typedef IPSet::iterator IPSetIter;

	typedef map<uint32_t,IPSet> ReplicasMap;
	typedef pair<uint32_t,IPSet> ReplicasPair;
	typedef ReplicasMap::iterator ReplicasMapIter;

	typedef map<uint32_t,uint32_t> LoadMap;
	typedef LoadMap::iterator LoadMapIter;
	typedef pair<uint32_t,uint32_t>LoadPair;

	typedef map<uint32_t,uint32_t> ShardMap;
	typedef pair<uint32_t,uint32_t>ShardPair;
	typedef ShardMap::iterator ShardMapIter;

	struct LoadLess{
		bool operator()(const LoadPair& lhs,const LoadPair& rhs){
			return lhs.second<rhs.second;
		}
	};

	struct LoadMore{
		bool operator()(const LoadPair& lhs,const LoadPair& rhs){
			return lhs.second>rhs.second;
		}
	};

	explicit GraphTable(uint32_t replicasNum):m_replicasNum(replicasNum){}

	IPSet select_replicas_hosts(){
		IPSet ips;
		vector<LoadPair> v;
		
		LoadMore loadMore;
		v.reserve(m_sloadMap.size());
		v.insert(v.end(),m_sloadMap.begin(),m_sloadMap.end());
		make_heap(v.begin(),v.end(),loadMore);
		do{
			uint32_t ip=v.front().first;
			ips.insert(ip);
			m_sloadMap[ip]++;
			pop_heap(v.begin(),v.end(),loadMore);
			v.pop_back();
		}while(ips.size()<m_replicasNum);
		return ips;

	}

	void deselect_replicas_hosts(const IPSet& ips){
		for(IPSetIter it=ips.begin();it!=ips.end();it++)m_sloadMap[*it]--;
	}

	uint32_t select_host(uint32_t sgkey){
		IPSet& ips=m_replicasMap[sgkey];
		LoadPair minpair={0,~0ul};
		LoadLess lt;
		for (IPSetIter it=ips.begin();it!=ips.end();it++){
			LoadPair tmppair={*it,m_aloadMap[*it]};
			if (lt(tmppair,minpair))minpair=tmppair;
		}
		return minpair.first;
	}

	//nameserv create a new subgraph 
	void create(uint32_t sgkey)
	{
		IPSet ips=select_replicas_hosts();
		for (IPSetIter it=ips.begin();it!=ips.end();it++){
			m_replicasMap[sgkey].insert(*it);
		}
		refresh(sgkey);
	}

	// nameservs destroy a subgraph
	void destroy(uint32_t sgkey)
	{
		if (m_replicasMap.count(sgkey)==0)return;
		deselect_replicas_hosts(m_replicasMap[sgkey]);
		m_aloadMap[m_shardMap[sgkey]]--;
		m_replicasMap.erase(sgkey);
		m_shardMap.erase(sgkey);
	}

	template<typename Iterator>
	void submit_sgkeys(uint32_t ip,Iterator begin,Iterator end)
	{
		m_sloadMap[ip]=end-begin;
		m_aloadMap[ip]=0;
		for (Iterator it=begin;it!=end;it++){
			m_replicasMap[*it].insert(ip);
			refresh(*it);
		}
	}
	bool exists(uint32_t sgkey){
		return m_replicasMap.count(sgkey)>0;
	}
	uint32_t get_replicas_num(uint32_t sgkey){
		if (m_replicasMap.count(sgkey)==0)return 0;
		else return m_replicasMap[sgkey].size();
	}
	template<typename Iterator>
	void get_replicas_hosts(uint32_t sgkey,Iterator begin,Iterator end){
		if (m_replicasMap.count(sgkey)==0)return;
		IPSet& ips=m_replicasMap[sgkey];
		std::copy(ips.begin(),ips.end(),begin);
	}
	size_t get_shard_table_size(){
		return m_replicasMap.size();
	}
	template<typename Iterator>
	void get_shard_table(Iterator begin,Iterator end){
		ShardMapIter it=m_shardMap.begin();
		Iterator it1=begin;
		for(;it!=m_shardMap.end()&&it1!=end;++it){
			if (it->second==0)continue;
			it1->sgkey=it->first;
			it1->ip=it->second;
			it1++;
		}
	}
	//debug
	ostream& dump(ostream& out){
		out<<"____replicas map____"<<endl;
		{
			ReplicasMapIter it0=m_replicasMap.begin();
			for (;it0!=m_replicasMap.end();it0++){
				out<<"[sgkey:"<<it0->first<<"]";
				IPSet& ips=it0->second;
				for (IPSetIter it1=ips.begin();it1!=ips.end();it1++){
					out<<"-->|host:"<<*it1<<"|";
				}
				out<<endl;
			}
		}

		out<<"____storage load map____"<<endl;	
		{
			LoadMapIter it=m_sloadMap.begin();
			for (;it!=m_sloadMap.end();it++){
				out<<"[host:"<<it->first<<"]="<<it->second<<endl;
			}
		}
		out<<"____access load map____"<<endl;
		{
			LoadMapIter it=m_aloadMap.begin();
			for (;it!=m_aloadMap.end();it++){
				out<<"[host:"<<it->first<<"]="<<it->second<<endl;
			}
		}
		out<<"____shard map____"<<endl;
		{
			ShardMapIter it=m_shardMap.begin();
			for (;it!=m_shardMap.end();it++){
				out<<"[sgkey:"<<it->first<<"]->[host:"<<it->second<<"]"<<endl;
			}
		}
		return out;
	}
private:
	//update partition
	void refresh(uint32_t sgkey){
		if (m_shardMap.count(sgkey)!=0)m_aloadMap[m_shardMap[sgkey]]--;
		uint32_t newip=select_host(sgkey);
		m_shardMap[sgkey]=newip;
		m_aloadMap[newip]++;
	}

	ReplicasMap m_replicasMap;
	LoadMap   m_sloadMap;
	LoadMap   m_aloadMap;
	ShardMap m_shardMap;
	uint32_t m_replicasNum;
};
}}
#endif
