// b) por meio do socket "listen" aceitar solicitações de conexão dos clientes, e estabelecer conexões com os clientes. 

// c) Fazer uso de programação de redes que lide com conexões simultâneas (por exemplo, por meio de multiprocess e multithreads). Ou seja, o servidor web deve poder receber solicitações de vários clientes ao mesmo
// tempo
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
	string port = "3000";
	string dir = "/tmp";

	if(argc == 4){
		host = argv[1];
		port = argv[2];
		dir = argv[3];
	} else if(argc == 3){
		host = argv[1];
		port = argv[2];
	} else if(argc == 2){
		host = argv[1];
	}

	cout << "ip address: " << getIP(host) << endl;

	// a) converter o nome do host do servidor em endereço IP, 
	// abrir socket para escuta neste endereço IP e no número 
	// de porta especificado. 


	return 0;
}