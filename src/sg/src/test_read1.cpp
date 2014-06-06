#include<nynn_mm_config.hpp>
#include<nynn_ipc.hpp>
int main()
{

	string fpath="data.0";
    MmapFile m(fpath);
    void* base=m.getBaseAddress();
    int *p1=(int*)base;	
	cout<<base<<"  "<<*p1<<endl;
    base=++p1;
    *p1=79;
}
