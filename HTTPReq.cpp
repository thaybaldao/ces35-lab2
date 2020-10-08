#include "HTTPReq.h"

HTTPReq::HTTPReq(string s){
	int startPos = 0, lastPos;
	lastPos = s.find(' ');
	this->method = s.substr(startPos, lastPos - startPos);

	startPos = lastPos + 1;
	lastPos = s.find(' ', startPos);
	this->URL = s.substr(startPos, lastPos - startPos);

	startPos = lastPos + 1;
	lastPos = s.find('\r', startPos);
    this->version = s.substr(startPos, lastPos - startPos);

    startPos = lastPos + 2;
    lastPos = s.find('\r', startPos);
    while(lastPos != string::npos){
    	this->headers.push_back(s.substr(startPos, lastPos - startPos));
    	startPos = lastPos + 2;
    	lastPos = s.find('\r', startPos);
    }

    this->headers.pop_back();
}


string HTTPReq::encode(){
	string req;

	req = this->method + " " + this->URL + " " + this->version + "\r\n";
	for(string h : this->headers){
		req += h + "\r\n";
	}

	req += "\r\n";

	return req;
}