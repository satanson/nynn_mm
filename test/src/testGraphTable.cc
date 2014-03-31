#include<nynn_mm_graph_table.hpp>
#include<nynn_mm_types.hpp>
using namespace nynn::mm;
void handle_submit_sgkeys(GraphTable& gt){
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
	gt.submit_sgkeys(ip,sgkeys.begin(),sgkeys.end());
	cout<<"submit_sgkeys [host:"<<ip<<"]";
	vector<uint32_t>::iterator it=sgkeys.begin();
	for (;it!=sgkeys.end();it++)cout<<"->||sgkey:"<<*it<<"||";
	cout<<endl;

}

void handle_create(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	gt.create(sgkey);
	cout<<"create [sgkey:"<<sgkey<<"]"<<endl;
}
void handle_destroy(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	gt.destroy(sgkey);
	cout<<"destroy [sgkey:"<<sgkey<<"]"<<endl;
}
void handle_get_replicas_hosts(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	vector<uint32_t> ipv(gt.get_replicas_num(sgkey),0);
	gt.get_replicas_hosts(sgkey,ipv.begin(),ipv.end());
	cout<<"get_replicas_hosts:[sgkey:"<<sgkey<<"]";
	vector<uint32_t>::iterator it=ipv.begin();
	for (;it!=ipv.end();it++)cout<<"->||host:"<<*it<<"||";
	cout<<endl;
}

void handle_exists(GraphTable& gt){
	uint32_t sgkey;
	cin>>sgkey;
	cout<<"[sgkey:"<<sgkey<<"] exists?" <<boolalpha<<gt.exists(sgkey)<<endl;
}

void handle_get_shard_table(GraphTable& gt){
	ShardTable& st=*ShardTable::make(gt.get_shard_table_size());
	gt.get_shard_table(&st[0],&st[0]+st.num());
	cout<<"get_shard_table:"<<endl;
	for (int i=0;i<st.num();i++){
		cout<<"[sgkey:"<<st[i].sgkey<<"]->[host:"<<st[i].targetip<<"]"<<endl;
	}
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
		if (func=="create")handle_create(gt);
		else if (func=="destroy")handle_destroy(gt);
		else if (func=="submit_sgkeys")handle_submit_sgkeys(gt);
		else if (func=="exists")handle_exists(gt);
		else if (func=="get_replicas_hosts")handle_get_replicas_hosts(gt);
		else if (func=="get_shard_table")handle_get_shard_table(gt);
		else{
			cout<<"UNKNOWN FUNCTION"<<endl;
		}
		gt.dump(cout);
	}
}
