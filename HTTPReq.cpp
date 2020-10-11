#include "HTTPReq.h"

/*
		Construtor da classe sem argumentos para que seja populada a partir
		da decodificação do lado do servidor.
*/
HTTPReq::HTTPReq(){}

 /*
 		Construtor da classe com argumentos. E utilizado para construir
		a mensagem com a requisicao que sera enviado ao servidor. Se
		utiliza essa funcao na requisicao partindo do cliente.
 */
HTTPReq::HTTPReq(string url, string host){
	this->method = "GET"; // sao apenas relizadas requisicoes do tipo GET
	this->URL = url;
	this->version = "HTTP/1.0"; // reproduzir comportamento de HTTP/1.0
	this->headers.push_back("Host: " + host); // acrescentar primeiro header
}


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

/*
		Funcao para codificar todo o conteudo do cabacalho HTTP em uma so string.
		Sera utilizado pelo cliente, que aplicara ainda uma conversao para o tipo
		unsigned char antes do envio.
*/
string HTTPReq::encode(){
	string req;

	// concatenar na string as partes iniciais do cabecalho
	req = this->method + " " + this->URL + " " + this->version + "\r\n";
	// percorrer vetor de headers para concatenar os elementos
	for(string h : this->headers){
		req += h + "\r\n";
	}

	req += "\r\n"; // adicionar "\r\n" para que o final do arquivo seja "\r\n\r\n"

	return req;
}
