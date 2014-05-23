#include<nynn_fs.hpp>
#include<nynn_file.hpp>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

int main(int argc,char**argv){
  
    string data[4]={"I like you","You like me","ok","let us marry"};
    uint32_t vtxno=1;
	nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
	nynn_file f(fs,vtxno,true);
	Block blk;
	CharContent *cctt=blk;
    int i=0;
	while(i<4){
		if (data[i].size()>CharContent::CONTENT_CAPACITY) data[i].resize(CharContent::CONTENT_CAPACITY);
		cctt->resize(data[i].size());
		std::copy(data[i].begin(),data[i].end(),cctt->begin());
		f.push(&blk);
        i++;
	}
}
