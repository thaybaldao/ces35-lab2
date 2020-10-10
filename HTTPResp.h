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
	int startContent;
	int contentLen;
	HTTPResp();
	HTTPResp(string status);
	void decode(unsigned char resp[]);
	vector<unsigned char> encode();
};