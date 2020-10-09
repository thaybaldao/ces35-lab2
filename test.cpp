#include <iostream>
#include "HTTPReq.h"
//#include "HTTPResp.h"

using namespace std;

int main(){
	//string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\nContent-Length: 50\r\n\r\nandasçdmasmd";
	// string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\nContent-Length: 50\r\n\r\n\r\n";
	string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\nContent-Length: 50\r\n\r\n";
	// string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\n\r\t";
	// string s = "GET /index.html HTTP/1.0\r\nHost: www.google.com\r\n\r\n";
	// string s = "GET /index.html HTTP/1.0\r\n\r\n";
	// string s = "GET /index.html HTTP/1.0\r\n";
	// string s = "GET /index.html HTTP/1.0";
	// string s = "GET /index.html \n";
	// string s = "GET /index.html";
	// string s = "";
	// string s = "\n";
	//string s = "\r\n\r\n";
	//string s = "   \r\n\r\n";

	// HTTPReq req = HTTPReq(s);
	// HTTPReq req2 = HTTPReq(req.encode());

	// cout << req2.method << endl;
	// cout << req2.URL << endl;
	// cout << req2.version << endl;
	
	// for(string s : req2.headers){
	// 	cout << s << endl;	
	// }

	//string s = "HTTP/1.1 200 OK\r\nDate: Sun, 26 Sep 2010 20:09:20 GMT\r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified: Tue, 30 Oct 2007 17:00:02 GMT\r\nETag: \"17dc6-a5c-bf716880\"\r\nAccept-Ranges: bytes\r\nContent-Length: 2652\r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=ISO-8859-1\r\n\r\ndata data data data data ...";
	//string s = "HTTP/1.1 200 OK\r\nDate: Sun, 26 Sep 2010 20:09:20 GMT\r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified: Tue, 30 Oct 2007 17:00:02 GMT\r\nETag: \"17dc6-a5c-bf716880\"\r\nAccept-Ranges: bytes\r\nContent-Length: 2652\r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=ISO-8859-1\r\n\r\n";
	//string s = "HTTP/1.1 200 OK\r\n\r\n";
	HTTPReq req = HTTPReq();

	if(req.decode(s)){
		cout << "Deu bom!" << endl;
		cout << req.method << endl;
		cout << req.URL << endl;
		cout << req.version << endl;
		
		for(string s : req.headers){
			cout << s << endl;	
		}
	} else {
		cout << "400 Bad request :(" << endl;
	}

	// cout << endl << endl;
	// HTTPResp res2 = HTTPResp(res.encode());


	return 0;
}