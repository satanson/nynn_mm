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
    cout<<base<<"  "<<*p1<<endl;
    base=p1+1;
    Edge* pe=(Edge*)base;
    int i=0;
    while(i<*p1){
        cout<<base<<"  "<<pe->m_sink<<" "<<pe->m_weight.m_fval<<" "<<pe->m_timestamp<<endl;
        base=++pe;
        i++;
    } 
    
}
