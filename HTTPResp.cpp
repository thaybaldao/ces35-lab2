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

/* 
    Metodo para decodificar uma response recebida pelo cliente
    Retorna um inteiro contendo o numero de bytes do conteudo
    da response que ainda nao foram lidos, pois os arquivos
    muito longos sao enviados em varios chuncks consecutivos
    e essa informacao de quantos bytes ainda precisam ser lidos 
    eh importante para que o client saiba quantas mensagens
    ainda precisa ler para obter o arquivo completo
*/
int HTTPResp::decode(unsigned char buf[], int bufSize){
    string s((char*) buf);

    // Procura o primeiro espaco presente na response para 
    // identificar a cadeia anterior a ele, que corresponde
    // a versao da response
	int startPos = 0, lastPos;
    lastPos = s.find(' ');
    if(lastPos == string::npos){ // se nao encontrar o espaco, bad response
        return -1;
    }
	this->version = s.substr(startPos, lastPos - startPos); // popula versao da response
    if(this->version.size() == 0){ // se nao houver versao, bad response
        return -1;
    }

    // Procura o primeiro \r\n presente na response para 
    // identificar a cadeia anterior a ele, que corresponde
    // ao status da response
	startPos = lastPos + 1;
	lastPos = s.find("\r\n", startPos);
    if(lastPos == string::npos){ // se nao encontrar o \r\n, bad response
        return -1;
    }
    this->status = s.substr(startPos, lastPos - startPos); // popula status da response
    if(this->status.size() == 0){ // se nao houver status, bad response
        return -1;
    }
    startPos = lastPos + 2; // pula \r\n

    // Procura o segundo \r\n presente na response para 
    // identificar um header ou fim dos headers
    lastPos = s.find("\r\n", startPos);
    if(lastPos == string::npos){ // se nao encontrar o \r\n, bad response
        return -1;
    }

    while(true){
        // Se a proxima cadeia iniciar com \r, eh porque encontramos o fim dos headers e 
        // o loop para encontrar headers pode encerrar
        if(s[startPos] == '\r'){
            startPos += 2;
            break;
        }
        // Do contrario, encontramos um header
    	this->headers.push_back(s.substr(startPos, lastPos - startPos)); // popula header da response
    	startPos = lastPos + 2; // pula \r\n

        // Procura o proximo \r\n presente na response para 
        // identificar um header ou fim dos headers
    	lastPos = s.find("\r\n", startPos);
        if(lastPos == string::npos){ // se nao encontrar o \r\n, bad response
            return -1;
        }
    }

    // Procura dentre os headers aquele que contem o tamanho do conteudo
    // da response e armazena o valor deste tamanho na variavel contentLen
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

    // Verifica se o restante desta response contem todo o conteudo enviado,
    // isto eh, verifica se a quantidade de bytes do conteudo (armazenado em
    // contentLen) equivale a quantidade de bytes que ainda restam ser lidos
    // da response. Caso ainda faltem bytes a serem lidos (os quais virao
    // nos proximos chunks), armazenar a quantidade de bytes que ainda
    // precisarao ser lidos na variavel nBytesLeft
    int nBytesLeft = 0;
    int chunkContentLen = 0;
    if(startPos + contentLen > bufSize){
        chunkContentLen = bufSize - startPos;
        nBytesLeft = contentLen - chunkContentLen;
    } else {
        chunkContentLen = contentLen;
    }

    // ler conteudo do buffer da response
    for(int i = startPos; i < startPos + chunkContentLen; ++i){
        this->content.push_back(buf[i]);
    }

    return nBytesLeft;
}

/*
    Metodo para codificar todo o conteudo de uma response HTTP em um vetor
    de unsigned char. Sera utilizado pelo servidor para enviar respostas 
    ao cliente.
*/
vector<unsigned char> HTTPResp::encode(){
    string respStr;

    // Concatena em uma string as partes iniciais da response
    respStr = this->version + " " + this->status + "\r\n";

    // Percorre vetor de headers para concatena-los na response
    for(string h : this->headers){
        respStr += h + "\r\n";
    }

    // Insere "\r\n" que demarca onde inicia o conteudo da response
    respStr += "\r\n";

    // Transforma string do cabecalho da response em um vector<unsigned char>,
    // pois o retorno do metodo nao eh uma string, mas sim um vector<unsigned char>
    vector<unsigned char> resp(respStr.begin(), respStr.end());

    // Concatena o conteudo ao cabecalho da response
    for(int i = 0; i < this->content.size(); ++i){
        resp.push_back(this->content[i]);
    }

    return resp;
}
