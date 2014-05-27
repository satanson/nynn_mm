#include<nynn_fs.hpp>
#include<nynn_file.hpp>
#include<fstream>
#include<iostream>
#include<string>
using namespace std;
using namespace nynn;
using namespace nynn::mm;
using namespace nynn::cli;

int main(int argc,char**argv){
    ifstream in("rfc2014.txt");
    string tmp;
    uint32_t vtxno=0;
    nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
    nynn_file f(fs,vtxno,true);
    Block blk;
    CharContent *cctt=blk;
    while(getline(in,tmp)){
		tmp+='\n';
        cctt->resize(tmp.size());
        std::copy(tmp.begin(),tmp.end(),cctt->begin());
		f.push(&blk);       	
    }
	in.close();
}





