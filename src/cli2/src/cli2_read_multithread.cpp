#include<nynn_fs.hpp>
#include<nynn_file.hpp>
using namespace nynn::cli;
using namespace std;

typedef unique_ptr<thread_t> Thread;
typedef unique_ptr<Thread[]> ThreadArray;

uint32_t thdsz;
string naddr,daddr;

void* reader(void*arg){
	int N=(intptr_t)arg;

	nynn_fs fs(naddr,daddr);
	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);
	uint32_t w=SubgraphSet::VERTEX_INTERVAL_WIDTH;
	uint32_t vtxno_beg=w*N;
	uint32_t vtxno_end=w*(N+1);
	uint32_t cnt=0;
	for (uint32_t vtxno=vtxno_beg;vtxno<vtxno_end && cnt<w;vtxno++) {
		nynn_file f(fs,vtxno);
		uint32_t blkno=f.getheadblkno();
		while(blkno!=nynn_file::invalidblkno){
			shared_ptr<Block> blk=f.read(blkno);
			assert(blk.get()!=NULL);
			blkno=blk->getHeader()->getNext();
			EdgeContent *ectt=*blk.get();
			concurrency+=ectt->end()-ectt->begin();
			++cnt;
			if (cnt%100000==0) cout<<cnt<<endl;
			nbytes+=sizeof(Block);
		}
	}
	clock_gettime(CLOCK_MONOTONIC,&end_ts);
	tbegin=begin_ts.tv_sec+begin_ts.tv_nsec/1.0e9;
	tend=end_ts.tv_sec+end_ts.tv_nsec/1.0e9;
	t=tend-tbegin;
	ofstream out(format("output.%d",pthread_self()));
	out.setf(ios::fixed);
	out<<"tbegin:"<<tbegin<<endl;
	out<<"tend:"<<tend<<endl;
	out<<"t:"<<t<<endl;
	out<<"nbytes:"<<nbytes<<endl;
	out<<"concurrency:"<<concurrency<<endl;
	out<<"throughput:"<<nbytes/1024.0/1024.0<<endl;
	out<<"ave_concurrency:"<<concurrency/t<<endl;
	out<<"ave_throughput:"<<nbytes/t/1024.0/1024.0<<endl;
	pthread_exit(NULL);
}
int main(int argc,char**argv)
{
	naddr=argv[1];
	daddr=argv[2];
	thdsz=parse_int(argv[3],16);

#ifdef SCHED
#pragma message "sched_setscheduler"
	int num_cpus=sysconf(_SC_NPROCESSORS_ONLN);
	log_i("num_cpus=%i",num_cpus);

	struct sched_param param;
	param.sched_priority=sched_get_priority_max(SCHED_RR);
	if(sched_setscheduler(0,SCHED_RR,&param))
		throw_nynn_exception(errno,"failed to sched_setscheduler");

	cpu_set_t *cpuset=CPU_ALLOC(num_cpus);
	size_t cpusetsz=CPU_ALLOC_SIZE(num_cpus);
	CPU_ZERO_S(cpusetsz,cpuset);
	CPU_SET_S(0,cpusetsz,cpuset);
	if(sched_setaffinity(0,cpusetsz,cpuset))
		throw_nynn_exception(errno,"failed to sched_setaffinity");
	CPU_FREE(cpuset);
#endif


	ThreadArray threads(new Thread[thdsz]);
	for(int i=0;i<thdsz;i++){
		threads[i].reset(new thread_t(reader,(void*)i));
#ifdef SCHED
#pragma message "sched_setscheduler"
		struct sched_param param;
		param.sched_priority=sched_get_priority_max(SCHED_RR);
		int rc=0;
		rc=pthread_setschedparam(threads[i]->thread_id(),SCHED_RR,&param);
		if (rc)throw_nynn_exception(rc,"failed to pthread_setschedparam");
		cpu_set_t *cpuset=CPU_ALLOC(num_cpus);
		size_t cpusetsz=CPU_ALLOC_SIZE(num_cpus);
		CPU_ZERO_S(cpusetsz,cpuset);
		CPU_SET_S((i+1)%num_cpus,cpusetsz,cpuset);
		rc=pthread_setaffinity_np(threads[i]->thread_id(),CPU_ALLOC_SIZE(num_cpus),cpuset);
		if (rc)throw_nynn_exception(rc,"failed to pthread_setaffinity_np");
		CPU_FREE(cpuset);
#endif
	}
	sleep(1);
	for (int i=0;i<thdsz;i++)threads[i]->start();
	for (int i=0;i<thdsz;i++)threads[i]->join();
	return 0;

}
