#include<iostream>
#include<nynn_common.hpp>
using namespace std;
using namespace nynn;

int main(){
	try{
		throw_nynn_exception(EINVAL,"invalid argument");
	}catch(nynn_exception_t& ex){
		cout<<ex.what()<<endl;
	}
}
