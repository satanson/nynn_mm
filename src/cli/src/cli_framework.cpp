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

	int loop=parse_int(argv[3],1<<10);
	zmq::context_t ctx;
	zmq::socket_t sock(ctx,ZMQ_REQ);
	string serv_endpoint="tcp://"+text_ip+":"+text_port;

	sock.connect(serv_endpoint.c_str());
	log_i("connected to %s",serv_endpoint.c_str());
	prot::Requester req(sock);

<<<<<<< HEAD
	struct timeval beg_tv,end_tv;
	double t;
	
	cout<<"loop="<<loop<<endl;
	long long int nbytes=0;
	
	gettimeofday(&beg_tv,NULL);
	for (int i=0;i<loop;i++){
		req.ask(0,NULL,0,NULL,0);
		req.parse_ans();
		nbytes+=req.get_data_size();
	}
	gettimeofday(&end_tv,NULL);
	t=((end_tv.tv_sec*1000+end_tv.tv_usec/1000)-(beg_tv.tv_sec*1000+beg_tv.tv_usec/1000))/1000.0;
	cout<<"time elapse:"<<t<<"s"<<endl;
	cout<<"nbytes:"<<nbytes<<"B"<<endl;
	cout<<"concurrency:"<<loop/t<<endl;
	cout<<"throughput:"<<nbytes/t/(1024.0*1024.0)<<" MB/s"<<endl;
=======
	struct timespec begin_ts,end_ts;
	double t;
	
	cout<<"loop="<<loop<<endl;

	uint64_t concurrency=0;
	uint64_t nbytes=0;
	double  tbegin=0;
	double  tend=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	
	for (int i=0;i<loop;i++){
		req.ask(0,NULL,0,NULL,0);
		req.parse_ans();
		concurrency++;
		nbytes+=req.get_data_size();
	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	cout.setf(ios::fixed);
	cout<<"tbegin:"<<tbegin<<endl;
	cout<<"tend:"<<tend<<endl;
	cout<<"t:"<<t<<endl;
	cout<<"nbytes:"<<nbytes<<endl;
	cout<<"concurrency:"<<concurrency<<endl;
	cout<<"throughput:"<<nbytes/1024.0/1024.0<<endl;
	cout<<"ave_concurrency:"<<concurrency/t<<endl;
	cout<<"ave_throughput:"<<nbytes/t/1024.0/1024.0<<endl;
>>>>>>> cef92da064ff2f8989a949b4ef422acc4d7d2310
}
