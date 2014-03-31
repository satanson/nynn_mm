#include<zmq.hpp>
#include<nynn_mm_common.h>
using namespace std;
using namespace nynn::mm::common;

int main(){
	zmq::context_t ctx;
	int major,minor,patch;
	zmq::version(&major,&minor,&patch);
	log_i("zmq version:%d.%d.%d",major,minor,patch);
}
