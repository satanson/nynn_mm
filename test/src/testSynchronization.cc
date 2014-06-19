#include<nynn_mm_common.h>

using namespace nynn::mm::common;

int n=0;
RWLock rwlock;
void* inc(void*arg)
{
	for (int i=0;i<10;i++){
		ExclusiveSynchronization es(&rwlock);
		sleep(rand_int()%4);
		n++;
		sleep(rand_int()%4);
		cout<<"thread#"<<pthread_self()<<":n="<<n<<endl;
	}
}

int main()
{
	pthread_t tid[2];
	for (int i=0;i<2;i++) {
		pthread_create(&tid[i],NULL,&inc,NULL);
	}
	for (int i=0;i<2;i++) {
		pthread_join(tid[i],NULL);
	}
}
