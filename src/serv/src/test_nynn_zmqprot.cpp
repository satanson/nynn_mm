#include<nynn_zmqprot.hpp>
using namespace std;
using namespace nynn;

void toUpperCase(char& c){
	c=toupper(c);
}
int port=20000;
void* serv(void* args){
	zmq::context_t& ctx=*(zmq::context_t*)args;
	zmq::socket_t sock(ctx,ZMQ_REP);
	string endpoint=string()+"tcp://192.168.1.11:"+to_string(port);
	sock.bind(endpoint.c_str());
	while(true){
		prot::replier rep(sock);
		rep.parse_ask();
		uint8_t cmd=rep.get_cmd();
		char* obeg=(char*)rep.get_options();
		char* oend=obeg+rep.get_options_size();
		char* dbeg=(char*)rep.get_data();
		char* dend=dbeg+rep.get_data_size();
		string os(obeg,oend),ds(dbeg,dend);
		std::for_each(ds.begin(),ds.end(),ptr_fun(toUpperCase));
		string s=string()+"options="+os+" data="+ds;
		rep.ans(true,s.c_str(),s.size());
	}
}
int main(){
	time_t t;
	time(&t);
	port=rand_r((uint32_t*)&t);
	string endpoint=string()+"tcp://192.168.1.11:"+to_string(port);
	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	sock.connect(endpoint.c_str());
	pthread_t id;
	pthread_create(&id,NULL,serv,&ctx);
	string line;
	while(getline(cin,line)){
		prot::requester req(sock);
		req.ask(1,NULL,0,line.c_str(),line.size());
		req.parse_ans();
		uint32_t status=req.get_status();
		char* beg=(char*)req.get_data();
		char* end=beg+req.get_data_size();
		string s(beg,end);	
		cout<<"status="<<status<<","
			<<"data="<<s<<endl;
	}
	pthread_join(id,NULL);
}
