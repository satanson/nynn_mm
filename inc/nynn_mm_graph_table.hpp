#ifndef NYNN_MM_GRAPH_TABLE_BY_SATANSON
#define NYNN_MM_GRAPH_TABLE_BY_SATANSON
#include<nynn_common.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_zmqprot.hpp>
using namespace std;
using namespace nynn;
namespace nynn{namespace mm{
class GraphTable{
public:
	typedef set<uint32_t> IPSet;
	typedef IPSet::iterator IPSetIter;
	typedef unordered_map<uint32_t,IPSet> ReplicasMap;
	typedef ReplicasMap::iterator ReplicasMapIter;

	typedef unordered_map<uint32_t,uint32_t> LoadMap;
	typedef LoadMap::iterator LoadMapIter;
	typedef pair<uint32_t,uint32_t> LoadPair;

	typedef unordered_map<uint32_t,uint32_t> ShardMap;
	typedef pair<uint32_t,uint32_t>ShardPair;
	typedef ShardMap::iterator ShardMapIter;

	struct Replicas{

		bool insert(uint32_t sgkey,uint32_t ip){ 
			m_replicasMap[sgkey].insert(ip); 
		}

		bool insert(uint32_t sgkey,vector<uint32_t>&ips){
			m_replicasMap[sgkey].insert(ips.begin(),ips.end());
		}

		bool erase(uint32_t sgkey,uint32_t ip){ 
			m_replicasMap[sgkey].erase(ip);
			if (m_replicasMap[sgkey].empty())m_replicasMap.erase(sgkey);
		}

		bool erase(uint32_t sgkey){
			m_replicasMap.erase(sgkey);
		}
		
		IPSet& operator[](uint32_t sgkey){ return m_replicasMap[sgkey];}
		
		ostream& dump(ostream& out){
			for (ReplicasMapIter it=m_replicasMap.begin();it!=m_replicasMap.end();it++){
				cout<<it->first<<": ";
				IPSet& ipset=it->second;
				for (IPSetIter it0=ipset.begin();it0!=ipset.end();it0++){
					cout<<"-->'"<<*it0<<"'";
				}
				cout<<endl;
			}
		}

	private:
		ReplicasMap m_replicasMap;
	};
	
	//ip->number of subgraphs
	struct Load{

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

		//constructor
		Load(vector<uint32_t>&ips){
			for (int i=0;i<ips.size();i++)m_loadMap[ips[i]]=0;
		}
		Load(){}

		//select replicasNum load least ips for storing replicas 
		vector<uint32_t> selectReplicHosts(uint32_t replicasNum) {
			vector<uint32_t> ips;
			ips.reserve(replicasNum);

			vector<LoadPair> v;
			v.reserve(m_loadMap.size());
			v.insert(v.end(),m_loadMap.begin(),m_loadMap.end());
			LoadMore loadMore;
			make_heap(v.begin(),v.end(),loadMore);
			do{
				uint32_t ip=v.front().first;
				ips.push_back(ip);
				m_loadMap[ip]++;
				pop_heap(v.begin(),v.end(),loadMore);
				v.pop_back();
			}while(ips.size()<replicasNum);
			return ips;
		}

		void deselectReplicHosts(vector<uint32_t>& ips){
			for (int i=0;i<ips.size();i++)m_loadMap[ips[i]]--;
		}

		uint32_t getLoadLeastHost(vector<uint32_t>& ips){
			vector<LoadPair> v;
			v.reserve(ips.size());

			for (int i=0;i<ips.size();i++){
				uint32_t ip=ips[i];			
				v.push_back(LoadPair(ip,m_loadMap[ip]));
			}

			uint32_t ip=min_element(v.begin(),v.end(),LoadLess())->first;
			m_loadMap[ip]++;
			return ip;
		}

		uint32_t& operator[](uint32_t ip){
			return m_loadMap[ip];
		}

		ostream& dump(ostream& out){
			for(LoadMapIter it=m_loadMap.begin();it!=m_loadMap.end();it++){
				cout<<it->first<<": "<<it->second<<endl;
			}
			return out;
		}
	private:
		LoadMap m_loadMap;
	};
	
	struct Partition{
		uint32_t& operator[](uint32_t sgkey){
			return m_partitionMap[sgkey];
		}

		void erase(uint32_t sgkey){
			m_partitionMap.erase(sgkey);
		}
		ostream& dump(ostream& out){
			for(ShardMapIter it=m_partitionMap.begin();it!=m_partitionMap.end();it++){
				out<<it->first<<"->"<<it->second<<endl;
			}
			return out;
		}
		ShardMap& getShardMap(){
			return m_partitionMap;
		}

		vector<ShardPair> getShards(){
			vector<ShardPair> pairs;
			pairs.reserve(m_partitionMap.size());
			ShardMapIter it=m_partitionMap.begin();
			while(it!=m_partitionMap.end()){
				if (it->second!=0)pairs.push_back(*it);
				it++;
			}
			return pairs;
		}
	private:
		ShardMap m_partitionMap;
	};
	
	GraphTable(vector<uint32_t>& ips,uint32_t replicasNum):
		m_sLoad(ips),m_aLoad(ips),m_replicasNum(replicasNum)
	{

	}
	explicit GraphTable(uint32_t replicasNum):m_replicasNum(replicasNum){}

	//nameserv create a new subgraph 
	void create(uint32_t sgkey)
	{
		vector<uint32_t> ips=m_sLoad.selectReplicHosts(m_replicasNum);
		for (int i=0;i<ips.size();i++){
			uint32_t ip=ips[i];
			m_replicas.insert(sgkey,ip);
		}
		refresh(sgkey);
	}

	// nameservs destroy a subgraph
	void destroy(uint32_t sgkey)
	{
		set<uint32_t>& ipSet=m_replicas[sgkey];
		vector<uint32_t> ips(ipSet.begin(),ipSet.end());
		m_sLoad.deselectReplicHosts(ips);
		m_aLoad[m_partition[sgkey]]--;
		m_replicas.erase(sgkey);
		m_partition.erase(sgkey);
	}
	// update data.

	//fault-tolerance: recycle corrupted subgraph
	uint32_t recycle(uint32_t sgkey,uint32_t ip)
	{
		m_replicas.erase(sgkey,ip);
		m_rottenReplicas.insert(sgkey,ip);
		m_sLoad[ip]--;
		if (ip==m_partition[sgkey]){
			refresh(sgkey);
		}
		return 0;
	}

	//fault-tolerance: restore corrupted subgraph
	uint32_t restore(uint32_t sgkey,uint32_t ip)
	{
		m_rottenReplicas.erase(sgkey,ip);
		m_replicas.insert(sgkey,ip);
		vector<uint32_t> ips;
		ips.push_back(ip);
		m_sLoad.getLoadLeastHost(ips);
		refresh(sgkey);
		return 0;
	}
	uint32_t recycleHost(uint32_t ip){
		
		return 0;
	}

	uint32_t restoreHost(uint32_t ip){
		return 0;
	}
	//recv dataservs's submission of subgraphkeys and write into graph table.
	uint32_t setAllSgkeysOfHost(uint32_t ip,vector<uint32_t>& sgkeys)
	{
		m_sLoad[ip]=sgkeys.size();
		m_aLoad[ip];
		for (int i=0;i<sgkeys.size();i++){
			uint32_t sgkey=sgkeys[i];
			m_replicas.insert(sgkey,ip);
			refresh(sgkey);
		}
		return 0;
	}
	//response a dataserv request for from which remote dataservs it fetch data.
	uint32_t getOptimalHostOfSgkey(uint32_t sgkey){
		return m_partition[sgkey];
	}

	IPSet& getAllHostsOfSgkey(uint32_t sgkey){
		return m_replicas[sgkey];
	}
	vector<ShardPair> getShards(){
		return m_partition.getShards();
	}
	//debug
	ostream& dump(ostream& out){
		out<<"____subgraph distribution____"<<endl;
		m_replicas.dump(out);
		out<<"____corrupted subgraph Distribution____"<<endl;
		m_rottenReplicas.dump(out);
		out<<"____storage load____"<<endl;
		m_sLoad.dump(out);
		out<<"____access load____"<<endl;
		m_aLoad.dump(out);
		out<<"____partition____"<<endl;
		m_partition.dump(out);
		return out;
	}
private:
	//update partition
	void refresh(uint32_t sgkey){
		uint32_t oldip=m_partition[sgkey];
		if (oldip!=0)m_aLoad[oldip]--;

		set<uint32_t>& ipSet=m_replicas[sgkey];
		vector<uint32_t> ips(ipSet.begin(),ipSet.end());
		uint32_t newip=m_aLoad.getLoadLeastHost(ips);
		m_partition[sgkey]=newip;
	}

	Replicas m_replicas;
	Replicas m_rottenReplicas;
	Load   m_sLoad;//storage load
	Load   m_aLoad;//access load
	Partition m_partition;
	uint32_t m_replicasNum;
};
}}
#endif
