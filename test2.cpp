#include <iostream>
#include <string>
#include <vector>
#include <string.h>

using namespace std;

int main(){
	unsigned char bufOut[9];
	vector<unsigned char> bufOutVec;
	for(int i = 0; i < 9; ++i){
		bufOutVec.push_back('c');
	}
	for(int i = 0; i < 9; ++i){
		bufOutVec.push_back('j');
	}
	
	cout << sizeof(bufOut) << endl;
	copy(bufOutVec.begin() + 0, bufOutVec.begin() + 9, bufOut);
	for(int i = 0; i < 9; ++i){
		cout <<  bufOut[i] << endl;
	}

	memset(bufOut, '\0', sizeof(bufOut));
	copy(bufOutVec.begin() + 9, bufOutVec.begin() + 18, bufOut);
	for(int i = 0; i < 9; ++i){
		cout <<  bufOut[i] << endl;
	}

	return 0;
}