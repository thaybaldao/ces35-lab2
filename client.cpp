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

/*
		Funcao para segmentar a string de entrada do client e separar
		os trechos de hostname, porta e arquivo alvo.
		Padrao de reconhecimento: http://hostname:port/file
*/
void handleURL(string& url, string& host, string& portStr, string& nameFile){
	int startPos = 7; // descontar "http://"
	int lastPos = url.find(':', startPos);
	host = url.substr(startPos, lastPos - startPos); // isolar nome do host

	startPos = lastPos + 1;
	lastPos = url.find('/', startPos);
	portStr = url.substr(startPos, lastPos - startPos); // isolar o numero da porta

	nameFile = url.substr(lastPos, url.size() - lastPos); // isolar nome do arquivo no final
}

/*
		Funcao para converter o hostname em um indereco de IP (IPv4)
*/
string getIP(string host){
	struct addrinfo hints;
	struct addrinfo* res;

	// configuracao do socket
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP

	// funcao de obtencao do endereco via DNS - getaddrinfo
	int status = 0;
	if ((status = getaddrinfo(host.c_str(), "80", &hints, &res)) != 0) {
		cerr << "getaddrinfo: " << gai_strerror(status) << endl;
		return "";
	}

	struct addrinfo* p = res;
	struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

	// conversao do endereco IP para string
	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

	freeaddrinfo(res); // libera memoria alocada
	return ipstr;
}

int main(int argc, char** argv) {
	vector<string> urls;

	// Retornar caso o numero de argumentos seja menor que 2
	if(argc < 2){
		return 1;
	}

	for(int i = 1; i < argc; ++i){
		/*
			Segmentar a string da URL e interpretar os parametros 
			da mesma como hostname, porta e o objeto a ser solicitado.
		*/
		urls.push_back(argv[i]);
		string host;
		string portStr;
		string nameFile;
		handleURL(urls[i-1], host, portStr, nameFile);
		
		// Converter a porta em numero inteiro
		stringstream ss;
		ss << portStr;
		int port;
		ss >> port;

		// Identificar IP do host
		string ip = getIP(host);

		/* O cliente se conecta por TCP com o servidor Web. */
		
		// Criar um socket para IPv4 usando protocolo de transporte TCP
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		// Preencher struct sockaddr_in com os dados do socket do servidor
		struct sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
		memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

		// Conectar com o servidor atraves do socket
		if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
			perror("connect");
			return 2;
		}

		// Obter o endereço IP do cliente e imprimi-lo
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
			perror("getsockname");
			return 3;
		}
		char ipstr[INET_ADDRSTRLEN] = {'\0'};
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		cout << endl << "##############################################" << endl;
		cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;

		/* 
			Assim que a conexão for estabelecida, o cliente constroi 
			uma solicitação HTTP e a envia ao servidor Web, ficando
			bloqueado aguardando uma resposta.
		*/
		unsigned char bufIn[2048] = {0}; // Buffer de entrada com 2048 bytes
		unsigned char bufOut[1024] = {0}; // Buffer de saida com 1024 bytes

		// Zerar a memoria do buffer
		memset(bufIn, '\0', sizeof(bufIn));
		memset(bufOut, '\0', sizeof(bufOut));

		// Criar requisicao HTTP GET e inseri-la no buffer de saida
		HTTPReq req = HTTPReq(nameFile, host);
		strcpy((char*)bufOut, req.encode().c_str());

		// Enviar requisicao
		if (send(sockfd, bufOut, sizeof(bufOut), 0) == -1) {
		  perror("send");
		  return 4;
		}

		// Receber conteudo no buffer de entrada
		if (recv(sockfd,  bufIn, sizeof(bufIn), 0) == -1) {
			perror("recv");
			return 5;
		}

		/*
			Apos receber a resposta, o cliente analisa se houve sucesso 
			ou falha por meio da analise do codigo de resposta. 
			Se houver sucesso, ele deve salvar o arquivo correspondente no 
			diretório atual usando o mesmo nome interpretado pela URL.
		*/
		HTTPResp resp = HTTPResp();
		/*
			Identifica o status e o header da resposta HTTP vindos na leitura do buffer,
			alem de quantos bytes do conteudo do arquivo retornado faltam ser lidos
		*/
		int nBytesLeft = resp.decode(bufIn, sizeof(bufIn));
		int nBytesReceived;

		/*
			Receber dados enquanto ainda ha conteudo do arquivo retornado
			pela resposta HTTP que ainda nao foi recebido
		*/
		while(nBytesLeft > 0){
			// Zerar memoria do buffer de entrada
			memset(bufIn, '\0', sizeof(bufIn));
			
			// Receber dados da resposta HTTP e inseri-los no seu conteudo
			nBytesReceived = recv(sockfd, bufIn, sizeof(bufIn), 0);
			if(nBytesReceived == -1){
				perror("recv");
				return 5;
			}
			for(int i = 0; i < nBytesReceived; ++i){
				resp.content.push_back(bufIn[i]);
			}

			// Atualizar numero de bytes do conteudo que faltam ser lidos
			nBytesLeft -= nBytesReceived;
		}

		cout << endl << "Response " << resp.status << endl;

		if(resp.status == "200 OK"){ // Se o status da resposta for 200 - OK
			if(nameFile == "/"){ // Se foi requisitado a URL "/", salvar arquivo com nome index.html
				nameFile = "index.html";
			} else {
				// Caso contrario, o nome do arquivo a ser salvo eh o que foi digitado apos o ultimo caractere '/'
				reverse(nameFile.begin(), nameFile.end());
				int pos = nameFile.find('/');
				nameFile = nameFile.substr(0, pos);
				reverse(nameFile.begin(), nameFile.end());
			}

			// Escrever todo o conteudo do arquivo obtido pela resposta HTTP no arquivo de saida
			cout << endl << "Generating file..." << endl;
			ofstream outputFile(nameFile);
			outputFile.write((char*)&resp.content[0], resp.content.size());
			outputFile.close();
			cout << "File stored with name " << nameFile << endl;
		}

		// Fechar o socket
		close(sockfd);
		cout << endl << "Closing connection with: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
		cout << "##############################################" << endl;
	}

	return 0;
}
