#include<nynn_mm_graph_table.h>
using namespace nynn::mm;

int main()
{
	vector<string> hosts;
	int n;
	cin>>n;
	hosts.reserve(n);
	string host;
	for (int i=0;i<n;i++){
		cin>>host;
		hosts.push_back(host);
	}
	
	vector<uint32_t> keys;
	int m;
	cin>>m;
	keys.reserve(m);
	uint32_t k;
	for (int i=0;i<m;i++){
		cin>>k;
		keys.push_back(k);
	}

	GraphTable::SGDist sgDist;
	GraphTable::Load sLoad(hosts),aLoad(hosts);
	vector<string> selHosts;
	GraphTable::Partition part;

	for (int i=0;i<keys.size();i++)
	{
		sLoad.selectReplicHosts(selHosts,3);
		sgDist.insert(keys[i],selHosts);
		set<string>& hostSet=sgDist.getHosts(keys[i]);
		selHosts.resize(0);
		selHosts.insert(selHosts.end(),hostSet.begin(),hostSet.end());
		part.set(keys[i],aLoad.selectHost(selHosts));
	}
	cout<<"----subgraph distribution-----"<<endl;
	sgDist.dump(cout);
	cout<<"----storage load----"<<endl;
	sLoad.dump(cout);
	cout<<"----access load----"<<endl;
	aLoad.dump(cout);
	cout<<"----partition----"<<endl;
	part.dump(cout);
}
