#include "HTTPReq.h"

HTTPReq::HTTPReq(){}

HTTPReq::HTTPReq(string url, string host){
	this->method = "GET";
	this->URL = url;
	this->version = "HTTP/1.0";
	this->headers.push_back("Host: " + host);
}

/* 
	Metodo para decodificar um request recebido pelo servidor
	Retorna true se a decodificacao do request for bem-sucedida
	Este retorno eh importante para identificar bad requests
*/
bool HTTPReq::decode(string s){
	// Verifica se a mensagem esta vazia e termina com \n
	if(s.size() == 0 or s.back() != '\n'){
		return false;
	}

	// Procura o primeiro espaco presente no request para 
	// identificar a cadeia anterior a ele, que corresponde
	// ao metodo do request
	int startPos = 0, lastPos;
	lastPos = s.find(' ');
	if(lastPos == string::npos){ // se nao encontrar o espaco, bad request
		return false;
	}
	this->method = s.substr(startPos, lastPos - startPos); // popula metodo do request
	if(this->method.size() == 0){ // se nao houver metodo, bad request
		return false;
	}
	startPos = lastPos + 1; // pula espaco

	// Procura o segundo espaco presente no request para 
	// identificar a cadeia anterior a ele, que corresponde
	// a URL do request
	lastPos = s.find(' ', startPos);
	if(lastPos == string::npos){ // se nao encontrar o espaco, bad request 
		return false;
	}
	this->URL = s.substr(startPos, lastPos - startPos); // popula URL do request
	if(this->URL.size() == 0){	// se nao houver URL, bad request
		return false;
	}
	startPos = lastPos + 1; // pula espaco

	// Procura o primeiro \r\n presente no request para 
	// identificar a cadeia anterior a ele, que corresponde
	// a versao do request
	lastPos = s.find("\r\n", startPos);
	if(lastPos == string::npos){ // se nao encontrar o \r\n, bad request
		return false;
	}
    this->version = s.substr(startPos, lastPos - startPos); // popula versao do request
    if(this->version.size() == 0){ // se nao houver versao, bad request
		return false;
	}
    					        
    startPos = lastPos + 2; // pula \r\n
    
    // Procura o segundo \r\n presente no request para 
	// identificar um header ou fim do request
    lastPos = s.find("\r\n", startPos);
    if(lastPos == string::npos){ // se nao encontrar o \r\n, bad request
		return false;
	}

	// Enquanto nao processar todo o request
    while(lastPos != string::npos){
    	// Se a cadeia entre o \n do ultimo "\r\n" e o \r do atual
    	// "\r\n" tiver tamanho maior que zero, encontramos um header
    	if(lastPos - startPos > 0){ 
    		this->headers.push_back(s.substr(startPos, lastPos - startPos)); // popula headers do request
    	} 
    	// Ja se a cadeia entre o \n do ultimo "\r\n" e o \r do atual
    	// "\r\n" tiver tamanho igual a zero, eh porque devemos estar
    	// no "\r\n" que demarca o fim do request. Portanto, se entre
    	// o \r atual e o fim do request nao houver apenas dois 
    	// caracteres (que correspondem a \r\n), isso eh um bad request
    	else if(lastPos + 2 != s.size()){
    		return false;
    	} 
    	// Do contrario, chegamos ao fim de um request valido
    	else {
    		return true;
    	}

    	startPos = lastPos + 2; // pula \r\n

    	// Procura o proximo \r\n presente no request para 
		// identificar um header ou fim do request
    	lastPos = s.find("\r\n", startPos);
    }

    return false;
}

/*
	Metodo para codificar todo o conteudo do request HTTP em uma so string.
	Sera utilizado pelo cliente para enviar requisicoes ao servidor, que 
	aplicara ainda uma conversao para um array unsigned char antes do envio.
*/
string HTTPReq::encode(){
	string req;

	// Concatena em uma string as partes iniciais do request
	req = this->method + " " + this->URL + " " + this->version + "\r\n";
	
	// Percorre vetor de headers para concatena-los no request
	for(string h : this->headers){
		req += h + "\r\n";
	}

	// Insere "\r\n" que demarca o fim do request
	req += "\r\n";

	return req;
}