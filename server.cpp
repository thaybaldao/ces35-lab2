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
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

#include "HTTPResp.h"
#include "HTTPReq.h"

#define BACKLOG 10 // Numero de requisicoes pendentes para a funcao "listen"

using namespace std;

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

/*
	Funcao que serah aplicada em threads e interpreta a requisicao que chega do 
	endereco clientAddr e foi enviada pelo socket clientSockfd. A string dir guarda 
	o diretorio de origem dos arquivos lidos e retornados na resposta HTTP caso
	o status seja 200. Essa funcao tambem constroi, envia a resposta HTTP e fecha 
	o socket clientSockfd. 
*/ 
void serveRequest(struct sockaddr_in clientAddr, int clientSockfd, string dir){
	// Preencher o endereço IP do cliente com um vetor de caracteres
	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	cout << endl << "Accepting connection from " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;

	unsigned char bufIn[1024] = {0}; // Buffer de entrada com 1024 bytes
	unsigned char bufOut[2048] = {0}; // Buffer de saida com 2048 bytes

	// Strings de status das respostas HTTPs possiveis
	string status200 = "200 OK";
	string status400 = "400 Bad Request";
	string status404 = "404 Not Found";

	// Zerar a memoria do buffer de entrada
	memset(bufIn, '\0', sizeof(bufIn));
	
	// Receber ateh 1024 bytes do cliente, numero este suficiente para armazenar uma requisicao GET
	if (recv(clientSockfd, bufIn, sizeof(bufIn), 0) == -1) {
	  	perror("recv");
	  	return;
	}

	string reqStr((char *)bufIn); // String para guardar a requisicao HTTP
	
	// Estruturas de requisicao e resposta HTTP
	HTTPReq req = HTTPReq();
	HTTPResp resp;

	if(req.decode(reqStr)){ // Caso nao haja problemas com a leitura da requisicao recebida
		string nameFile = dir;
		if(req.URL == "/"){ // Se a requisicao chamar a URL "/", envia-se o arquivo index.html
			nameFile += "/index.html";
		} else{
			nameFile += req.URL;
		}

		// Abrir arquivo requisitado
		ifstream myFile(nameFile);
		if(!myFile.is_open()){ // Se nao encontrar o arquivo, retornar erro 404 - Not Found
			resp = HTTPResp(status404);
			// Preencher header da resposta HTTP com tamanho do conteudo do arquivo HTML retornado 
			resp.headers.push_back("Content-Length: " + to_string(resp.content.size()));
		} else { // Caso o arquivo exista, retornar resposta 200 - OK
			resp = HTTPResp(status200);
			
			/*
				Ler arquivo correspondente e salva-lo em um vetor de unsigned char,
				que serah o conteudo do arquivo da resposta HTTP
			*/
			stringstream aux;
		    aux << myFile.rdbuf();
		    vector<unsigned char> content{istreambuf_iterator<char>{aux}, istreambuf_iterator<char>{}};		
		    myFile.close();
			resp.content = content;

			// Preencher header da resposta HTTP com tamanho do conteudo do arquivo HTML retornado
		    resp.headers.push_back("Content-Length: " + to_string(content.size()));
		}
	} else { // Se ha problemas com a leitura da requisicao recebida, retornar erro 400 - Bad Request
		resp = HTTPResp(status400);
		// Preencher header da resposta HTTP com tamanho do conteudo do arquivo HTML retornado 
		resp.headers.push_back("Content-Length: " + to_string(resp.content.size()));
	}

	// Vetor de unsigned char que contem todo o conteudo da resposta HTTP
	vector<unsigned char> bufOutVec = resp.encode();
	int counter = bufOutVec.size(); // Numero de bytes que serao transmitidos no total
	int nBytesSent = 0; // Numero de bytes enviados para o cliente
	/*
		Iteradores auxiliares para percorrer o vetor bufOutVec 
		a ser enviado em multiplas chamadas da funcao send()
	*/
	int s = 0, e; 

	/*
		Enviar dados enquanto ainda ha conteudo 
		da resposta HTTP que ainda nao foi enviado
	*/
	while(counter > 0){
		// Zerar memoria do buffer de saida
		memset(bufOut, '\0', sizeof(bufOut));

		if(counter > sizeof(bufOut)){ // Se o numero de bytes que faltam ser enviados eh maior do que a capacidade do buffer de saida, serah enviada essa quantidade de dados
			e = s + sizeof(bufOut);
		} else { // Caso contrario, sera enviado apenas o numero de bytes necessario para terminar o envio da resposta HTTP
			e = s + counter;
		}

		// Colocar parte do vetor bufOutVec no buffer de saida
		copy(bufOutVec.begin() + s, bufOutVec.begin() + e, bufOut);

		/*
			Enviar buffer, atualizar o numero de bytes que faltam ser enviados e
			o indice de inicio de leitura do vetor bufOutVec
		*/
		nBytesSent = send(clientSockfd, bufOut, sizeof(bufOut), 0);
		if(nBytesSent == -1){
			perror("send");
		  	return;
		}
		counter -= nBytesSent;
		s = e;
	}

	cout << endl << "Returning response " << resp.status << " to " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;

	// Fechar o socket do cliente
	close(clientSockfd);
	cout << endl << "Closing connection with " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
}


int main(int argc, char** argv) {
	// Valores padroes de host, porta e diretorio de leitura de arquivos
	string host = "localhost";
	string portStr = "3000";
	string dir = "/tmp";

	// Atualizar valores de host, porta e diretorio de leitura de arquivos para os valores digitados pelo usuario
	if(argc == 4){
		host = argv[1];
		portStr = argv[2];
		dir = argv[3];
	} else if(argc == 3){
		host = argv[1];
		portStr = argv[2];
	} else if(argc == 2){
		host = argv[1];
	}

	// Converter a porta em numero inteiro
	stringstream ss;
	ss << portStr;
	int port;
	ss >> port;

	/*
		Converter o nome do host do servidor em endereço IP, 
		abrir socket para escuta neste endereço IP e no número 
		de porta especificado.
	*/

	string ip = getIP(host);
	
	// Criar um socket para IPv4 e usando protocolo de transporte TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/*
		Configurar socket para ele nao ficar preso e indisponível 
		por algum tempo apos seu fechamento.
	*/
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	// Preencher struct sockaddr_in com os dados do socket do servidor
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	// Registrar a porta para uso do socket
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	/*
		Por meio do socket "listen", aceitar solicitações de
		conexão dos clientes e estabelecer conexões com eles.
	*/

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		return 3;
	}

	while(true){
		// Aceitar a conexao TCP com o cliente
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSockfd == -1) {
			perror("accept");
			return 4;
		}

		/*
			Fazer uso de programação de redes que lide com conexões simultâneas por 
			multithreads. Ou seja, o servidor web pode receber solicitações 
			de vários clientes ao mesmo tempo.
		*/
		thread(serveRequest, clientAddr, clientSockfd, dir).detach();
	}

	return 0;
}