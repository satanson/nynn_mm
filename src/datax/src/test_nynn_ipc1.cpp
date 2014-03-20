#include<nynn_common.hpp>
#include<sched.h>
using namespace std;
using namespace nynn;

Monitor m;
int cnt=0;
int incm(){
	int i=cnt;
	sched_yield();
	rand_nanosleep();
	cnt++;
	return i;
}
void* foobar(void* args){
	for (int i=0;i<10;i++){
		auto tmp=spinsync<int>(m,incm);
		//auto tmp=incm();
		cout<<tmp<<endl;
	}
}

int main(){
	pthread_t tids[2];
	for (int i=0;i<2;i++){
		pthread_create(tids+i,NULL,foobar,NULL);
	}
	for (int i=0;i<2;i++){
		pthread_join(tids[i],NULL);
	}
}
