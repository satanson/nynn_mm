#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_zmqprot.hpp>
#include<nynn_mm_types.hpp>
#include<nynn_mm_config.hpp>
#include<nynn_mm_handler.hpp>

int main(int argc,char**argv)
{

	string text_ip=argv[1];
	string text_port=argv[2];
	int loop=parse_int(argv[3],1<<11);
	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	string serv_endpoint="tcp://"+text_ip+":"+text_port;
	sock.connect(serv_endpoint.c_str());
	log_i("connected to %s",serv_endpoint.c_str());
	prot::Requester req(sock);

	struct timeval beg_tv,end_tv;
	double t;
	
	char blk[1<<16];
	cout<<"loop="<<loop<<endl;
	gettimeofday(&beg_tv,NULL);
	for (int i=0;i<loop;i++){
		req.ask(0,NULL,0,NULL,0);
		req.parse_ans();
		//memcpy(blk,req.get_data(),req.get_data_size());
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"throughput:"<<loop*(1<<16)/t/(1024.0*1024.0)<<" MB/s"<<endl;
}
