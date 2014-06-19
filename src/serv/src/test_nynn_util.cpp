#include<nynn_common.hpp>
using namespace std;
using namespace nynn;

int main(){
	string host=get_host();
	uint32_t ip=get_ip();
	cout<<"host="<<host<<endl;
	cout<<"ip="<<ip2string(ip)<<endl;
	cout<<string("pi=")+to_string(3.1415926)<<endl;
	cout<<string("N=")+to_string(13)<<endl;
	
	cout<<time2string(time(NULL))<<endl;
}
