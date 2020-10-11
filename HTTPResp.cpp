#include "HTTPResp.h"

/*
		Construtor da classe sem argumentos para que seja aplicado
		metodo de decodificação do lado do cliente.
*/
HTTPResp::HTTPResp(){}

/*
   Construtor da classe com argumentos. E utilizado para construir
   a mensagem com conteudo que sera retornado ao cliente. Se utiliza
   essa funcao na resposta do servidor.
*/
HTTPResp::HTTPResp(string status){
    this->version = "HTTP/1.0"; // fixar HTTP/1.0
    this->status = status; // inserir o status recebido como argumento
    this->headers = {}; // iniciar vetor de headers
    // inicializar content padrao para retorno
    string content = "<!DOCTYPE html>\n<html>\n<head>\n<title>CES-35 Lovers</title>\n</head>\n<body>\n<h1>"+status+"</h1>\n</body>\n</html>";
    vector<unsigned char> contentConverted(content.begin(), content.end());
    this->content = contentConverted; // preencher content em formato adequado de unsigned char 
}

int HTTPResp::decode(unsigned char buf[], int bufSize){
    string s((char*) buf);
	int startPos = 0, lastPos;
	lastPos = s.find(' ');
	this->version = s.substr(startPos, lastPos - startPos);

	startPos = lastPos + 1;
	lastPos = s.find('\r', startPos);
    this->status = s.substr(startPos, lastPos - startPos);

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

    int nBytesLeft = 0;
    int chunkContentLen = 0;
    if(startPos + contentLen > bufSize){
        chunkContentLen = bufSize - startPos;
        nBytesLeft = contentLen - chunkContentLen;
    } else {
        chunkContentLen = contentLen;
    }

    for(int i = startPos; i < startPos + chunkContentLen; ++i){
        this->content.push_back(buf[i]);
    }

    return nBytesLeft;
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

    resp.push_back('\0');

    return resp;
}
