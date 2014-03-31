#include<nynn_common.hpp>
#include<sched.h>
using namespace std;
using namespace nynn;
class MyCount{
	public:
		MyCount():cnt(0){}
		int inc(){
			int i=cnt;
			sched_yield();
			rand_nanosleep();
			cnt++;
			return i;
		}
	private:
		int cnt;
};

Monitor m;
void* foobar(void* args){
	MyCount& mycnt=*(MyCount*)args;
	auto inc=&MyCount::inc;
	for (int i=0;i<10;i++){
		//auto tmp=mfspinsync<int>(m,mycnt,inc);
		auto tmp=(mycnt.*inc)();
		cout<<tmp<<endl;
	}
}

int main(){
	MyCount mycnt;
	pthread_t tids[2];
	for (int i=0;i<2;i++){
		pthread_create(tids+i,NULL,foobar,&mycnt);
	}
	for (int i=0;i<2;i++){
		pthread_join(tids[i],NULL);
	}
}
