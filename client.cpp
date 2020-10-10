#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "HTTPResp.h"
#include "HTTPReq.h"

using namespace std;

void handleURL(string& url, string& host, string& portStr, string& nameFile){
	int startPos = 7;
	int lastPos = url.find(':', startPos);
	host = url.substr(startPos, lastPos - startPos);

	startPos = lastPos + 1;
	lastPos = url.find('/', startPos);
	portStr = url.substr(startPos, lastPos - startPos);

	nameFile = url.substr(lastPos, url.size() - lastPos);
}

string getIP(string host){
	struct addrinfo hints;
	struct addrinfo* res;

	// hints - modo de configurar o socket para o tipo  de transporte
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP

	// funcao de obtencao do endereco via DNS - getaddrinfo 
	// funcao preenche o buffer "res" e obtem o codigo de resposta "status" 
	int status = 0;
	if ((status = getaddrinfo(host.c_str(), "80", &hints, &res)) != 0) {
		cerr << "getaddrinfo: " << gai_strerror(status) << endl;
		return "";
	}

	struct addrinfo* p = res;
	// a estrutura de dados eh generica e portanto precisa de type cast
	struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

	// e depois eh preciso realizar a conversao do endereco IP para string
	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

	freeaddrinfo(res); // libera a memoria alocada dinamicamente para "res"
	return ipstr;
}

int main(int argc, char** argv) {
	vector<string> urls;

	if(argc < 2){
		return 1;
	}

	for(int i = 1; i < argc; ++i){
		urls.push_back(argv[i]);

		/* 1) segmentar a string da URL e interpretar os parametros 
		da mesma como hostname, porta e o objeto a ser solicitado.*/
		string host;
		string portStr;
		string nameFile;
		handleURL(urls[i-1], host, portStr, nameFile);

		/* 2) o cliente precisa se conectar por TCP com o servidor Web. */
		stringstream ss;
		ss << portStr;
		int port;
		ss >> port;

		string ip = getIP(host);
		
		// cria um socket para IPv4 e usando protocolo de transporte TCP
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		// struct sockaddr_in {
		//  short            sin_family;   // e.g. AF_INET, AF_INET6
		//  unsigned short   sin_port;     // e.g. htons(3490)
		//  struct in_addr   sin_addr;     // see struct in_addr, below
		//  char             sin_zero[8];  // zero this if you want to
		// };

		struct sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);     // porta tem 16 bits, logo short, network byte order
		serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
		memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

		// conecta com o servidor atraves do socket
		if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
			perror("connect");
			return 2;
		}

		// a partir do SO, eh possivel obter o endereço IP usando 
		// pelo cliente (nos mesmos) usando a funcao getsockname
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
			perror("getsockname");
			return 3;
		}

		// em caso de multiplos endereços, isso permite o melhor controle
		// a rotina abaixo, imprime o valor do endereço IP do cliente
		char ipstr[INET_ADDRSTRLEN] = {'\0'};
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		cout << endl << "##############################################" << endl;
		cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;

		/* 3) Assim que a conexão for estabelecida, o cliente precisa 
		construir uma solicitação HTTP e enviar ao servidor Web e 
		ficar bloqueado aguardando uma resposta. */
		unsigned char bufIn[2097152] = {0};
		unsigned char bufOut[1024] = {0};

		// zera a memoria do buffer
		memset(bufIn, '\0', sizeof(bufIn));
		memset(bufOut, '\0', sizeof(bufOut));

		HTTPReq req = HTTPReq(nameFile, host);

		strcpy((char*)bufOut, req.encode().c_str());

		// converte a string lida em vetor de bytes 
		// com o tamanho do vetor de caracteres
		if (send(sockfd, bufOut, sizeof(bufOut), 0) == -1) {
		  perror("send");
		  return 4;
		}

		// // recebe no buffer uma certa quantidade de bytes
		if (recv(sockfd,  bufIn, sizeof(bufIn), 0) == -1) {
			perror("recv");
			return 5;
		}

		// /* 4) Após receber a resposta, o cliente precisa analisar se 
		// houve sucesso ou falha, por meio de análise do código de resposta. 
		// Se houver sucesso, ele deve salvar o arquivo correspondente no 
		// diretório atual usando o mesmo nome interpretado pela URL. */
		HTTPResp resp = HTTPResp();
		resp.decode(bufIn);

		cout << endl << "Response " << resp.status << endl;

		if(resp.status == "200 OK"){
			if(nameFile == "/"){
				nameFile = "index.html";
			} else {
				reverse(nameFile.begin(), nameFile.end());
				int pos = nameFile.find('/');
				nameFile = nameFile.substr(0, pos);
				reverse(nameFile.begin(), nameFile.end());
			}

			cout << endl << "Generating file..." << endl;
			// ofstream outputFile(nameFile, ios::out | ios::binary);
			// outputFile.write((const char*)&resp.content[0], resp.content.size());
			// outputFile.close();
			FILE * pFile;
			pFile = fopen (nameFile.c_str(), "wb");
			fwrite (&bufIn[resp.startContent] , sizeof(unsigned char), resp.contentLen, pFile);
			fclose (pFile);

			cout << "File stored with name " << nameFile << endl;
		}

		// fecha o socket
		close(sockfd);
		cout << endl << "Closing connection with: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
		cout << "##############################################" << endl;
	}

	return 0;
}
