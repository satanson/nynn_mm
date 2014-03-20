#include<nynn_mm_graph_table.hpp>
using namespace nynn::mm;
void handle_setAllSgkeysOfHost(GraphTable& gt){
	uint32_t ip;
	cin>>ip;
	uint32_t n,sgkey;
	cin>>n;
	vector<uint32_t> sgkeys;
	sgkeys.reserve(n);

	for(int i=0;i<n;i++){
		cin>>sgkey;
		sgkeys.push_back(sgkey);
	}
	gt.setAllSgkeysOfHost(ip,sgkeys);
}
void handle_getOptimalHostOfSgkey(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	cout<<"getOptimalHostOfSgkey: "<<sgkey<<"->"<<gt.getOptimalHostOfSgkey(sgkey)<<endl;
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
void handle_getAllHostsOfSgkey(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	set<uint32_t> ips=gt.getAllHostsOfSgkey(sgkey);
	cout<<"getAllHostsOfSgkey: "<<sgkey<<": ";
	set<uint32_t>::iterator it=ips.begin();
	for (;it!=ips.end();it++)cout<<"->"<<*it<<endl;
	cout<<endl;
}

void handle_recycle(GraphTable& gt){
	uint32_t sgkey;
	uint32_t ip;
	cin>>sgkey>>ip;
	gt.recycle(sgkey,ip);
}

void handle_restore(GraphTable& gt){
	uint32_t sgkey;
	uint32_t ip;
	cin>>sgkey>>ip;
	gt.restore(sgkey,ip);
}
int main(int argc,char**argv)
{
	vector<uint32_t> ips;
	int n;
	cin>>n;
	ips.reserve(n);
	uint32_t ip;
	for (int i=0;i<n;i++){
		cin>>ip;
		ips.push_back(ip);
	}
	GraphTable gt(3);
	string func;

	while(cin>>func){
		
		cout<<func<<endl;
		if (func=="setAllSgkeysOfHost")handle_setAllSgkeysOfHost(gt);
		else if (func=="getOptimalHostOfSgkey")handle_getOptimalHostOfSgkey(gt);
		else if (func=="create")handle_create(gt);
		else if (func=="destroy")handle_destroy(gt);
		else if (func=="getAllHostsOfSgkey")handle_getAllHostsOfSgkey(gt);
		else if (func=="recycle")handle_recycle(gt);
		else if (func=="restore")handle_restore(gt);
		else{
			cout<<"UNKNOWN FUNCTION"<<endl;
		}
		gt.dump(cout);
	}
}
