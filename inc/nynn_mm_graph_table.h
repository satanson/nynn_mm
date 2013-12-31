#ifndef NYNN_MM_GRAPH_TABLE_BY_SATANSON
#define NYNN_MM_GRAPH_TABLE_BY_SATANSON
#include<nynn_mm_common.h>
using namespace std;
using namespace nynn::mm::common;
namespace nynn{namespace mm{
class GraphTable{
public:
	typedef struct SubgraphDistribution{
		typedef set<string> HostSet;
		typedef HostSet::iterator HostSetIter;
		typedef map<uint32_t,HostSet> SG2HostsMap;
		typedef SG2HostsMap::iterator SG2HostsMapIter;

		bool insert(uint32_t sgkey,const string& host) 
		{ 
			m_sg2hosts[sgkey].insert(host); 
		}
		bool insert(uint32_t sgkey,vector<string>&hosts)
		{
			m_sg2hosts[sgkey].insert(hosts.begin(),hosts.end());
		}
		bool erase(uint32_t sgkey,const string& host) 
		{ 
			m_sg2hosts[sgkey].erase(host);
			if (m_sg2hosts[sgkey].empty())m_sg2hosts.erase(sgkey);
		}
		bool erase(uint32_t sgkey)
		{
			m_sg2hosts.erase(sgkey);
		}
		HostSet& getHosts(uint32_t sgkey){ return m_sg2hosts[sgkey]; }
		
		ostream& dump(ostream& out){
			for (SG2HostsMapIter it=m_sg2hosts.begin();it!=m_sg2hosts.end();it++){
				cout<<it->first<<": ";
				HostSet& hostset=it->second;
				for (HostSetIter it0=hostset.begin();it0!=hostset.end();it0++){
					cout<<"-->'"<<*it0<<"'";
				}
				cout<<endl;
			}
		}

	private:
		SG2HostsMap m_sg2hosts;
	}SGDist;
	
	struct Load{
		typedef map<string,uint32_t> LoadMap;
		typedef LoadMap::iterator LoadMapIter;
		typedef pair<string,uint32_t> LoadPair;
		struct LoadLess{
			bool operator()(const LoadPair& lhs,const LoadPair& rhs)
			{
				return lhs.second<rhs.second;
			}
		};

		struct LoadMore{
			bool operator()(const LoadPair& lhs,const LoadPair& rhs)
			{
				return lhs.second>rhs.second;
			}
		};

		Load(vector<string>&hosts)
		{
			for (int i=0;i<hosts.size();i++)m_host2sgnum[hosts[i]]=0;
		}

		vector<string>& selectReplicHosts(vector<string>& hosts,uint32_t replica)
		{
			hosts.reserve(replica);
			hosts.resize(0);
			vector<LoadPair> v;
			v.reserve(m_host2sgnum.size());
			v.insert(v.end(),m_host2sgnum.begin(),m_host2sgnum.end());
			LoadMore loadMore;
			make_heap(v.begin(),v.end(),loadMore);
			do{

				LoadPair loadPair=v.front();
				string host=loadPair.first;
				hosts.push_back(host);
				m_host2sgnum[host]++;
				pop_heap(v.begin(),v.end(),loadMore);
				v.pop_back();
			}while(hosts.size()<replica);
			return hosts;
		}

		void deselectReplicHosts(vector<string>& hosts)
		{
			for (int i=0;i<hosts.size();i++)m_host2sgnum[hosts[i]]--;
		}

		string selectHost(vector<string>& hosts)
		{
			vector<LoadPair> v;
			v.reserve(hosts.size());
			v.resize(0);
			for (int i=0;i<hosts.size();i++){
				string host=hosts[i];			
				v.push_back(LoadPair(host,m_host2sgnum[host]));
			}
			string host=min_element(v.begin(),v.end(),LoadLess())->first;
			m_host2sgnum[host]++;
			return host;
		}
		void plus(const string& host,uint32_t n){
			m_host2sgnum[host]+=n;
		}
		void minus(const string& host, uint32_t n){
			m_host2sgnum[host]-=n;
		}	
		void deselectHost(const string& host){
			m_host2sgnum[host]--;
		}

		ostream& dump(ostream& out){
			for(LoadMapIter it=m_host2sgnum.begin();it!=m_host2sgnum.end();it++){
				cout<<it->first<<": "<<it->second<<endl;
			}
			return out;
		}
	private:
		LoadMap m_host2sgnum;
	};
	
	struct Partition{
		typedef map<uint32_t,string> PartitionMap;
		typedef PartitionMap::iterator PartitionMapIter;
		void set(uint32_t sgkey,const string& host){
			m_sg2host[sgkey]=host;
		}
		string get(uint32_t sgkey){
			if (m_sg2host.find(sgkey)!=m_sg2host.end()) {
				return m_sg2host[sgkey];
			}else{
				return "UNKNOWN";
			}
		}
		void erase(uint32_t sgkey){
			m_sg2host.erase(sgkey);
		}
		ostream& dump(ostream& out){
			for(PartitionMapIter it=m_sg2host.begin();it!=m_sg2host.end();it++){
				out<<it->first<<"->"<<it->second<<endl;
			}
			return out;
		}
	private:
		PartitionMap m_sg2host;
	};
	
	GraphTable(vector<string>& hosts,uint32_t replica):
		m_sLoad(hosts),m_aLoad(hosts),m_replica(replica)
	{

	}
	// update data.
	int32_t create(uint32_t sgkey)
	{
		vector<string> hosts;
		m_sLoad.selectReplicHosts(hosts,m_replica);
		for (int i=0;i<hosts.size();i++){
			string host=hosts[i];
			m_sgDist.insert(sgkey,host);
		}
		refresh(sgkey);
		return 0;
	}

	// update data.
	int32_t destroy(uint32_t sgkey)
	{
		set<string>& hostSet=m_sgDist.getHosts(sgkey);
		vector<string> hosts(hostSet.begin(),hostSet.end());
		m_sLoad.deselectReplicHosts(hosts);
		m_aLoad.deselectHost(m_partition.get(sgkey));
		m_sgDist.erase(sgkey);
		m_partition.erase(sgkey);
		return 0;
	}
	// update data.
	vector<string>& getHosts(uint32_t sgkey,vector<string>& hosts)
	{
		set<string>& hostSet=m_sgDist.getHosts(sgkey);

		hosts.resize(0);
		hosts.reserve(hostSet.size());
		hosts.insert(hosts.end(),hostSet.begin(),hostSet.end());
		
		return hosts;
	}

	//fault-tolerance: recycle corrupted subgraph
	uint32_t recycle(uint32_t sgkey,const string& host)
	{
		m_sgDist.erase(sgkey,host);
		m_corruptedSgDist.insert(sgkey,host);
		m_sLoad.deselectHost(host);
		if (host==m_partition.get(sgkey)){
			m_aLoad.deselectHost(host);
			refresh(sgkey);
		}
		return 0;
	}

	//fault-tolerance: restore corrupted subgraph
	uint32_t restore(uint32_t sgkey,const string& host)
	{
		m_corruptedSgDist.erase(sgkey,host);
		m_sgDist.insert(sgkey,host);
		vector<string> hosts;
		hosts.push_back(host);
		m_sLoad.selectHost(hosts);
		refresh(sgkey);
		return 0;
	}
	uint32_t recycleHost(const string& host){
		
		return 0;
	}

	uint32_t restore(const string& host){
		return 0;
	}

	//invoked by agent
	uint32_t setAllSgkeys(const string& host,vector<uint32_t>& sgkeys)
	{
		m_sLoad.plus(host,sgkeys.size());
		for (int i=0;i<sgkeys.size();i++){
			uint32_t sgkey=sgkeys[i];
			m_sgDist.insert(sgkey,host);
			refresh(sgkey);
		}
		return 0;
	}

	//invoked by agent
	string getHost(uint32_t sgkey){
		return m_partition.get(sgkey);
	}

	//debug
	ostream& dump(ostream& out){
		out<<"____subgraph distribution____"<<endl;
		m_sgDist.dump(out);
		out<<"____corrupted subgraph Distribution____"<<endl;
		m_corruptedSgDist.dump(out);
		out<<"____storage load____"<<endl;
		m_sLoad.dump(out);
		out<<"____access load____"<<endl;
		m_aLoad.dump(out);
		out<<"____partition____"<<endl;
		m_partition.dump(out);
		return out;
	}

private:
	
	void refresh(uint32_t sgkey){
		string oldHost=m_partition.get(sgkey);
		if (oldHost!="UNKNOWN")m_aLoad.minus(oldHost,1);

		set<string>& hostSet=m_sgDist.getHosts(sgkey);
		vector<string> hosts(hostSet.begin(),hostSet.end());
		m_partition.set(sgkey,m_aLoad.selectHost(hosts));
	}

	SGDist m_sgDist;
	SGDist m_corruptedSgDist;
	Load   m_sLoad;//storage load
	Load   m_aLoad;//access load
	Partition m_partition;
	uint32_t m_replica;
};
}}
#endif
