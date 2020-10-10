#include <iostream>
#include <sstream>
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
	int decode(unsigned char buf[], int bufSize);
	vector<unsigned char> encode();
};