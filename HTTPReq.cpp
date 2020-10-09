#include "HTTPReq.h"

HTTPReq::HTTPReq(){}

bool HTTPReq::decode(string s){
	if(s.size() == 0 or s.back() != '\n'){
		return false;
	}

	int startPos = 0, lastPos;
	lastPos = s.find(' ');
	if(lastPos == string::npos){
		return false;
	}
	this->method = s.substr(startPos, lastPos - startPos);
	if(this->method.size() == 0){
		return false;
	}

	startPos = lastPos + 1;
	lastPos = s.find(' ', startPos);
	if(lastPos == string::npos){
		return false;
	}
	this->URL = s.substr(startPos, lastPos - startPos);
	if(this->URL.size() == 0){
		return false;
	}

	startPos = lastPos + 1;
	lastPos = s.find('\r', startPos);
	if(lastPos == string::npos){
		return false;
	}
    this->version = s.substr(startPos, lastPos - startPos);
    if(this->version.size() == 0){
		return false;
	}
    					        
    startPos = lastPos + 2;
    lastPos = s.find('\r', startPos);
    if(lastPos == string::npos){
		return false;
	}

    while(lastPos != string::npos){
    	if(lastPos - startPos > 0){
    		this->headers.push_back(s.substr(startPos, lastPos - startPos));
    	} else if(lastPos + 2 != s.size()){
    		return false;
    	} else {
    		return true;
    	}

    	startPos = lastPos + 2;
    	lastPos = s.find('\r', startPos);
    }

    return false;
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