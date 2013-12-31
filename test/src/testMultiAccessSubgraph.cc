#include<nynn_mm_subgraph_storage.h>
#include<test.h>
using namespace nynn::mm;
static int const N=1000;
void* worker(void* arg)
{
	try{
		int M=N;
		Subgraph *sg=static_cast<Subgraph*>(arg);
		uint32_t blknos[N];
		for(int i=0;i<N;i++){
			blknos[i]=sg->require();
			if (blknos[i]==INVALID_BLOCKNO){
				M=i;
				break;
			}
			cout<<"thread#"<<pthread_self()<<"block#"<<blknos[i]<<":require"<<endl;
		}
		for(int i=0;i<M;i++){
			Block blk;
			sg->readBlock(blknos[i],&blk);
			cout<<"thread#"<<pthread_self()<<"block#"<<blknos[i]<<":readBlock"<<endl;
			sg->writeBlock(blknos[i],&blk);
			cout<<"thread#"<<pthread_self()<<"block#"<<blknos[i]<<":writeBlock"<<endl;
		}
		for(int i=0;i<M;i++){
			sg->release(blknos[N-1-i]);
			cout<<"thread#"<<pthread_self()<<"block#"<<blknos[i]<<":release"<<endl;
		}
	}catch(NynnException &err){
		err.printBacktrace();
	}
}

int main(int argc,char**argv)
{
	string path(argv[1]);
	try{
		Subgraph s(path);
		pthread_t tids[10];
		int K=10;
		for (int i=0;i<10;i++){
			int errnum=pthread_create(&tids[i],NULL,&worker,&s);
			if (errnum!=0){
				log_w("Fail to create thread#%d",i);
				K=i;
			}
		}
		for (int i=0;i<K;i++){
			pthread_join(tids[i],NULL);
		}

	}catch(NynnException &err){
		err.printBacktrace();
	}	
}
