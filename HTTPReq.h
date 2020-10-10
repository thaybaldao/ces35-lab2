#include <iostream>
#include <string>
#include <vector>

using namespace std;

class HTTPReq{
public:
	string method;
	string URL;
	string version;
	vector<string> headers;
	HTTPReq();
	HTTPReq(string url, string host);
	bool decode(string s);
	string encode();
};