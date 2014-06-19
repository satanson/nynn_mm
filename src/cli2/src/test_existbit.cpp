#include<nynn_mm_config.hpp>
#include<nynn_mm_types.hpp>
#include<linuxcpp.hpp>
using namespace std;
using namespace nynn::mm;
int main(){
	Vertex vtx(0);
	vtx.resetExistBit();
	cout<<vtx.getExistBit()<<endl;
	vtx.setExistBit();
	cout<<vtx.getExistBit()<<endl;
	vtx.resetExistBit();
	cout<<vtx.getExistBit()<<endl;
}
