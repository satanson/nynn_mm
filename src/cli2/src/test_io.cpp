#include<fstream>
#include<iostream>
#include<string>
using namespace std;
int main()
{
	ifstream in("1.txt");
    ofstream out("2.txt");
    string tmp;
    while(getline(in,tmp)){
		out<<tmp<<'\n';
    }
    in.close();
    out.close();
    
}
