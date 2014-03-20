#include<linuxcpp.hpp>
#include<nynn_common.hpp>
using namespace std;
using namespace nynn;

int main(){
	string host=get_host();
	cout<<"get_host: "<<host<<endl;
	cout<<"get_ip: "<<get_ip()<<endl;
	uint32_t ip=host2ip(host);
	cout<<"host2ip: "<<ip2string(ip)<<endl;
	cout<<"ip2host: "<<ip2host(ip)<<endl;

	string hosts[4]={"centos1","centos2","centos3","centos4"};
	try{
	for (int i=0;i<4;i++){
		cout<<hosts[i]<<"=> host2ip: "<<ip2string(host2ip(hosts[i]))
			<<" host: "<<ip2host(host2ip(hosts[i]))
			<<" ip: "<<ip2string(host2ip(ip2host(host2ip(hosts[i]))))
			<<endl;
	}
	}catch(nynn_exception_t& ex){
		cerr<<ex.what()<<endl;
	}
}
