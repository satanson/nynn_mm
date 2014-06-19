#include<nynn_mm_subgraph_storage.h>
#include<test.h>
unique_ptr<GraphCache> cache(new GraphCache());
uint32_t hitN=0;
uint32_t accessN=0;
Monitor hitMonitor;
Monitor accessMonitor;
void incHit()
{
	Synchronization s(&hitMonitor);
	hitN++;
}
void incAccess()
{
	Synchronization s(&accessMonitor);
	accessN++;
}

void* worker(void* arg)
{
	uint32_t* LMN=static_cast<uint32_t*>(arg);
	uint32_t  L=LMN[0];
	uint32_t  M=LMN[1];
	uint32_t  N=LMN[2];
	Block blk;
	for (uint32_t i=0;i<L;i++){
		uint32_t vtxno=rand_int()%M;
		uint32_t blkno=rand_int()%N;
		if (!cache->read(vtxno,blkno,&blk)){
			for (uint32_t i=0;i<8;i++) {
				sprintf(reinterpret_cast<char*>(&blk),"%d#%d",vtxno,blkno+i);
				cout<<"thread#"<<pthread_self()
					<<" cache:"<<reinterpret_cast<char*>(&blk)<<endl;
				cache->write(vtxno,blkno+i,&blk);
			}
		}else{
			incHit();
			cout<<"thread#"<<pthread_self()
				<<" read:"<<reinterpret_cast<char*>(&blk)<<endl;
		}
		incAccess();
	}
}

int main(int argc,char**argv)
{
	uint32_t LMNs[300];
	uint32_t K=0;
	while((cin>>LMNs[K++]) && K<300);

	pthread_t tids[100];
	for(uint32_t i=0;i<K/3;i++){
		int errnum=pthread_create(&tids[i],NULL,&worker,LMNs+3*i);
		if (errnum!=0){
			log_w("Fail to create thread#%d",i);
		}
	}
	for(uint32_t i=0;i<K/3;i++){
		pthread_join(tids[i],NULL);
	}
	cout<<"hit rate="<<(hitN+0.0)/accessN<<endl;
}
