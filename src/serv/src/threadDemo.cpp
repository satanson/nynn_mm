#define _GLIBCXX_USE_NANOSLEEP

#include<iostream>
#include<thread>
#include<chrono>
#include<random>
#include<cstdlib>
#include<atomic>
using namespace std;

int i=0;
void* func(){
	
	while(true){
		int tmpi=i++;
		cout<<tmpi<<endl;
		uint32_t seed=std::chrono::system_clock::now().time_since_epoch().count();
		srand(seed);
		this_thread::sleep_for(std::chrono::milliseconds(rand()%500));
	}
}
int main(){
	thread thds[2];
	thds[0]=thread(func);
	thds[1]=thread(func);
	this_thread::sleep_for(std::chrono::milliseconds(1000));
	thds[0]=thread();
	thds[1]=thread();
	thds[0].join();
	thds[1].join();
}
