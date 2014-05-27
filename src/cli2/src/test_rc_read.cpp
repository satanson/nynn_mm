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
    ofstream out("3.txt");
    uint32_t vtxno=0;
    nynn_fs fs("192.168.255.114:50000","192.168.255.114:60000");
    nynn_file f(fs,vtxno,true);
    Block blk;
    CharContent *cctt=blk;
    uint32_t blkno=nynn_file::headblkno;
    while(blkno!=nynn_file::invalidblkno){
		if(!f.read(blkno,&blk)) break;
        blkno=(blk.getHeader()->getNext)();
        string tmp(cctt->begin(),cctt->end());
        out<<tmp;
    }
	out.close();
}





