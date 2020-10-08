#include "HTTPResp.h"

HTTPResp::HTTPResp(){}

HTTPResp::HTTPResp(string s){
	int startPos = 0, lastPos;
	lastPos = s.find(' ');
	this->version = s.substr(startPos, lastPos - startPos);
    //cout << "version: " << this->version << endl;

	startPos = lastPos + 1;
	lastPos = s.find('\r', startPos);
    this->status = s.substr(startPos, lastPos - startPos);
    //cout << "status: " << this->status << endl;

    startPos = lastPos + 2;
    lastPos = s.find('\r', startPos);
    while(true){
        if(s[startPos] == '\r'){
            startPos += 2;
            break;
        }
    	this->headers.push_back(s.substr(startPos, lastPos - startPos));
    	startPos = lastPos + 2;
    	lastPos = s.find('\r', startPos);
    }

    //cout << "hearders: " << endl;
    // for(string s : this->headers){
    //     cout << s << endl;  
    // }

    //int size =  s.size();

    this->content = s.substr(startPos, s.size() - startPos);
    //cout << "content: " << this->content << endl;
    
}

string HTTPResp::encode(){
    string req;

    req = this->version + " " + this->status + "\r\n";
    for(string h : this->headers){
        req += h + "\r\n";
    }

    req += "\r\n" + this->content;

    return req;
}
