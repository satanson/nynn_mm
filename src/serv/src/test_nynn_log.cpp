#include<iostream>
#include<nynn_log.h>
#include<unistd.h>
using namespace std;
using namespace nynn::log;
int main(){
	log_i("hello world");
	log_w("hello %s","ranpanf");
	log_e(EINVAL);
	log_a(0!=0,"0 must be equal to 0");
}
