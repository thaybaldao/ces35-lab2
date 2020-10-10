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

#include "HTTPResp.h"
#include "HTTPReq.h"

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


int main(int argc, char** argv) {
	string host = "localhost";
	string portStr = "3000";
	string dir = "/tmp";

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

	stringstream ss;
	ss << portStr;
	int port;
	ss >> port;

	string ip = getIP(host);

	/* a) converter o nome do host do servidor em endereço IP, 
	 abrir socket para escuta neste endereço IP e no número 
	 de porta especificado. */
	
	// cria um socket para IPv4 e usando protocolo de transporte TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Opções de configuração do SOCKETs
	// No sistema Unix um socket local TCP fica preso e indisponível 
	// por algum tempo após close, a não ser que configurado SO_REUSEADDR
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	// struct sockaddr_in {
	//  short            sin_family;   // e.g. AF_INET, AF_INET6
	//  unsigned short   sin_port;     // e.g. htons(3490)
	//  struct in_addr   sin_addr;     // see struct in_addr, below
	//  char             sin_zero[8];  // zero this if you want to
	// };

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);     // porta tem 16 bits, logo short, network byte order
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	// realizar o bind (registrar a porta para uso com o SO) para o socket
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	/* b) por meio do socket "listen" aceitar solicitações de conexão dos clientes, 
	e estabelecer conexões com os clientes. */
	// colocar o socket em modo de escuta, ouvindo a porta 
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}

	// aceitar a conexao TCP
	// verificar que sockfd e clientSockfd sao sockets diferentes
	// sockfd eh a "socket de boas vindas"
	// clientSockfd eh a "socket diretamente com o cliente"
	while(true){
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSockfd == -1) {
			perror("accept");
			return 4;
		}

		/* c) Fazer uso de programação de redes que lide com conexões simultâneas (por 
		exemplo, por meio de multiprocess e multithreads). Ou seja, o servidor web 
		deve poder receber solicitações de vários clientes ao mesmo tempo */

		// usa um vetor de caracteres para preencher o endereço IP do cliente
		char ipstr[INET_ADDRSTRLEN] = {'\0'};
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		cout << endl << "##############################################" << endl;
		cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;

		// faz leitura e escrita dos dados da conexao
		unsigned char bufIn[1024] = {0};
		unsigned char bufOut[2097152] = {0};

		string status200 = "200 OK";
		string status400 = "400 Bad Request";
		string status404 = "404 Not Found";

		// zera a memoria do buffer
		memset(bufIn, '\0', sizeof(bufIn));
		memset(bufOut, '\0', sizeof(bufOut));

		// recebe ate 1024 bytes do cliente remoto
		if (recv(clientSockfd, bufIn, sizeof(bufIn), 0) == -1) {
		  	perror("recv");
		  	return 5;
		}

		string reqStr((char *)bufIn);
		HTTPReq req = HTTPReq();
		HTTPResp resp;

		if(req.decode(reqStr)){
			string nameFile = dir;
			if(req.URL == "/"){
				nameFile += "/index.html";
			} else{
				nameFile += req.URL;
			}

			ifstream myFile(nameFile);
			if(!myFile.is_open()){
				resp = HTTPResp(status404);
				resp.headers.push_back("Content-Length: " + to_string(resp.content.size()));
			} else {
				resp = HTTPResp(status200);
			
				stringstream aux;
			    aux << myFile.rdbuf(); //read the file
			    vector<unsigned char> content{istreambuf_iterator<char>{aux}, istreambuf_iterator<char>{}};	
			    for(unsigned char c : content){
				    cout << c;
				}
    	
			    myFile.close();

			    resp.content = content;

			    resp.headers.push_back("Content-Length: " + to_string(content.size()));
			}
		} else {
			resp = HTTPResp(status400);
			resp.headers.push_back("Content-Length: " + to_string(resp.content.size()));
		}

		vector<unsigned char> bufOutVec = resp.encode();
		copy(bufOutVec.begin(), bufOutVec.end(), bufOut);

		cout << endl << "Returning response " << resp.status << endl;

		// envia de volta o buffer recebido como um echo
		if (send(clientSockfd, bufOut, sizeof(bufOut), 0) == -1) {
			perror("send");
		  	return 6;
		}

		// fecha o socket
		close(clientSockfd);
		cout << endl << "Closing connection with: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
		cout << "##############################################" << endl;
	}

	return 0;
}