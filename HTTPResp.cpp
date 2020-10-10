#include "HTTPResp.h"

HTTPResp::HTTPResp(){}

HTTPResp::HTTPResp(string status){
    this->version = "HTTP/1.0";
    this->status = status;
    this->headers = {};
    string content = "<!DOCTYPE html>\n<html>\n<head>\n<title>CES-35 Lovers</title>\n</head>\n<body>\n<h1>"+status+"</h1>\n</body>\n</html>";
    vector<unsigned char> contentConverted(content.begin(), content.end());
    this->content = contentConverted;
}

void HTTPResp::decode(unsigned char resp[]){
    string s((char*) resp);
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

    int contentLen = 0;
    for(string h : this->headers){
        if(h.find("Content-Length:") != string::npos){
            int pos = h.find(" ");
            pos += 1;
            stringstream ss;
            ss << h.substr(pos, h.size() - pos);
            ss >> contentLen;
            break;
        }
    }

    //cout << "hearders: " << endl;
    for(string h : this->headers){
        if(h.find("Content-Length:") != string::npos){
            int pos = h.find(" ");
            pos += 1;
            stringstream ss;
            ss << h.substr(pos, h.size() - pos);
            ss >> this->contentLen;
            cout << contentLen << endl;
            break;
        }
    }


    for(int i = startPos; i < startPos + contentLen; ++i){
        this->content.push_back(resp[i]);
    }
    cout << "startPos: " << resp[startPos] << endl;
    this->startContent = startPos;

    stringstream aux;
    for(int i = startPos; i < startPos + contentLen ; ++i){
        aux << resp[i];
    }
    vector<unsigned char> cont{istreambuf_iterator<char>{aux}, istreambuf_iterator<char>{}}; 
    this->content = cont;
    
    // cout << "content: " << endl;
    // for(int i = startContent; i < startContent + contentLen; ++i){
    //     cout << resp[i];
    // }
    
}

vector<unsigned char> HTTPResp::encode(){
    string respStr;

    respStr = this->version + " " + this->status + "\r\n";
    for(string h : this->headers){
        respStr += h + "\r\n";
    }

    respStr += "\r\n";

    vector<unsigned char> resp(respStr.begin(), respStr.end());

    for(int i = 0; i < this->content.size(); ++i){
        resp.push_back(this->content[i]);
    }

    //resp.push_back('\0');

    // cout << "content: " << endl;
    // for(unsigned char c : this->content){
    //     cout << c;
    // }

    return resp;
}

