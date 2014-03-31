#include<iostream>
#include<vector>
#include<chrono>
#include<random>
#include<algorithm>
#include<nynn_mm_common.h>

using namespace std;

#if 1
template <typename C> ostream& print(ostream& out,const C& c)
{
	for(typename C::const_iterator it=c.cbegin();it!=c.cend();it++){
		out<<*it<<",";
	}
	return out;
};
#endif 
int main()
{
	vector<int> vct(10,0);
	for (int i=0;i<10;i++)vct[i]=i;

	for (int i=0;i<10;i++){
		uint32_t seed=std::chrono::system_clock::now().
			time_since_epoch().count();
		std::random_shuffle(vct.begin(),vct.end());
		print(cout,vct);
		cout<<endl;
	}
}
