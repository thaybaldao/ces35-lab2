#include <iostream>
#include <string>
#include <vector>

using namespace std;

class HTTPResp{
public:
	string version;
	string status;
	vector<string> headers;
	string content;
	HTTPResp();
	HTTPResp(string s);
	string encode();
};