#include<linuxcpp.hpp>
#include<nynn_mm_config.hpp>
typedef unique_ptr<thread_t> Thread;
typedef unique_ptr<Thread[]> ThreadArray;


uint32_t vtxno_beg;
uint32_t vtxno_end;
uint32_t loop;
uint32_t thdsz;

void* reader(void*arg){
	SubgraphSet &sgs=*(SubgraphSet*)arg;

	Block blk,*retblk;
	CharContent *content=blk;

	struct timespec begin_ts,end_ts;
	double tbegin,tend,t;
	uint64_t concurrency=0;
	uint64_t nbytes=0;
	clock_gettime(CLOCK_MONOTONIC,&begin_ts);

	for (int i=0;i<loop;i++)
	for (uint32_t vtxno=vtxno_beg;vtxno<vtxno_end;vtxno++) {
	
		uint32_t blkno=HEAD_BLOCKNO;
		while (blkno!=INVALID_BLOCKNO){
			retblk=sgs.read(vtxno,blkno,&blk);
			if (unlikely(retblk==NULL)) return 0;
			blkno=retblk->getHeader()->getNext();
#ifdef LOG
			string s(content->begin(),content->end());
			cout<<">"<<s<<endl;
#endif
			concurrency++;
			nbytes+=sizeof(Block);
		};
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
}

int main(int argc,char**argv)
{
	string basedir=argv[1];
	vtxno_beg=parse_int(argv[2],0);
	vtxno_end=parse_int(argv[3],1024);
	thdsz=parse_int(argv[4],16);
	loop=parse_int(argv[4],16);

	SubgraphSet sgs(basedir);

	vector<int32_t> sgkeys(sgs.get_sgkey_num(),0);
	sgs.get_sgkeys(sgkeys.begin(),sgkeys.end());
	for (int i=0;i<sgkeys.size();i++){
		sgs.attachSubgraph(sgkeys[i]);
	}

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
		threads[i].reset(new thread_t(reader,&sgs));
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
