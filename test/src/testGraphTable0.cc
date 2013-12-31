#include<nynn_mm_graph_table.h>
using namespace nynn::mm;
void handle_setAllSgkeys(GraphTable& gt){
	string host;
	cin>>host;
	uint32_t n,sgkey;
	cin>>n;
	vector<uint32_t> sgkeys;
	sgkeys.reserve(n);

	for(int i=0;i<n;i++){
		cin>>sgkey;
		sgkeys.push_back(sgkey);
	}
	gt.setAllSgkeys(host,sgkeys);
}
void handle_getHost(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	cout<<"getHost: "<<sgkey<<"->"<<gt.getHost(sgkey)<<endl;
}

void handle_create(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	gt.create(sgkey);
}
void handle_destroy(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	gt.destroy(sgkey);
}
void handle_getHosts(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	vector<string> hosts;
	gt.getHosts(sgkey,hosts);
	cout<<"getHosts: "<<sgkey<<": ";
	for (int i=0;i<hosts.size();i++)cout<<"->"<<hosts[i];
	cout<<endl;
}

void handle_recycle(GraphTable& gt){
	uint32_t sgkey;
	string host;
	cin>>sgkey>>host;
	gt.recycle(sgkey,host);
}

void handle_restore(GraphTable& gt){
	uint32_t sgkey;
	string host;
	cin>>sgkey>>host;
	gt.restore(sgkey,host);
}
int main(int argc,char**argv)
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
	GraphTable gt(hosts,3);
	string func;
	int K=atoi(argv[1]),k=0;
	while(cin>>func){
		
		cout<<func<<endl;
		if (func=="setAllSgkeys")handle_setAllSgkeys(gt);
		else if (func=="getHost")handle_getHost(gt);
		else if (func=="create")handle_create(gt);
		else if (func=="destroy")handle_destroy(gt);
		else if (func=="getHosts")handle_getHosts(gt);
		else if (func=="recycle")handle_recycle(gt);
		else if (func=="restore")handle_restore(gt);
		else{
			cout<<"UNKNOWN FUNCTION"<<endl;
		}
		gt.dump(cout);
	}
}
