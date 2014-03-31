#include<iostream>
#include<memory>
#include<nynn_log.hpp>
#include<nynn_thread.hpp>
#include<nynn_exception.hpp>
using namespace std;

static pthread_key_t flag_key;
void* func(void*args){
	int i=(intptr_t)args;
	
	pthread_setspecific(flag_key,(void*)1);
	int flag=1;
	cout<<"thread "<<i<<" is created"<<endl;
	while(flag){
		sleep(1);
		cout<<"thread "<<i<<" is runnning...."<<endl;
		flag=(intptr_t)pthread_getspecific(flag_key);
	}
	cout<<"thread "<<i<<"is terminated"<<endl;
}
void kill_thread(int signum){
	pthread_setspecific(flag_key,(void*)0);
}
void add_signal_handler(int signum,void(*handler)(int)){
	struct sigaction sigact;
	sigact.sa_handler=handler;
	sigaction(signum,&sigact,NULL);
}
class thread_key_t{
public:
	thread_key_t(pthread_key_t* k,void(*destructor)(void*)):key(k){
		pthread_key_create(key,destructor);
	}
	~thread_key_t(){pthread_key_delete(*key);}
private:
	pthread_key_t * key;
};

#define N 3
int main(){

	add_signal_handler(SIGTERM,&kill_thread);
	add_signal_handler(SIGINT,SIG_IGN);
	thread_key_t create(&flag_key,NULL);

	unique_ptr<thread_t> threads[N];
	for (int i=0;i<N;i++)threads[i].reset(new thread_t(func,(void*)i));
	for (int i=0;i<N;i++)threads[i]->start();

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGQUIT);
	cout<<"wait for 'SIGQUIT' to terminate process"<<endl;
	int signum;
	sigwait(&sigs,&signum);
	cout<<"process terminated by 'SIGQUIT'"<<endl;

	for (int i=0;i<N;i++){sleep(1);threads[i]->kill(SIGTERM);};
	sleep(1);
	for (int i=0;i<N;i++){if (threads[i]->is_alive())threads[i]->stop();}
	for (int i=0;i<N;i++){threads[i]->join();};
}
