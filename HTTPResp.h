#include <iostream>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

class HTTPResp{
public:
	string version;
	string status;
	vector<string> headers;
	vector<unsigned char> content;
	HTTPResp();
	HTTPResp(string status);
	//void decode(string s);
	vector<unsigned char> encode();
};