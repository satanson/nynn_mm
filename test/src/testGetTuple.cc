#include<nynn_mm_common.h>
using namespace nynn::mm::common;

int main(int argc,char**argv){
	ifstream out(argv[1]);
	vector<string> words;
	gettuple(out,words);
	for(int i=0;i<words.size();i++)cout<<words[i]<<endl;
}
