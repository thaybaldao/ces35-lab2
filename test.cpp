#include <iostream>
#include "HTTPReq.h"

using namespace std;

int main(){
	string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\nContent-Length: 35\r\n\r\n";
	//string s = "GET /index.html HTTP/1.0\r\n\r\n";
	HTTPReq req = HTTPReq(s);
	HTTPReq req2 = HTTPReq(req.encode());

	cout << req2.method << endl;
	cout << req2.URL << endl;
	cout << req2.version << endl;
	
	for(string s : req2.headers){
		cout << s << endl;	
	}

	return 0;
}