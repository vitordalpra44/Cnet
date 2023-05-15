#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

#define ISVALIDSOCKET(s) ((s) >=0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#include<stdio.h>
#include<string.h>
#include<time.h>

int main(){
	printf("\nConfigurando endereço local");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	struct addrinfo *bind_address;
	getaddrinfo(0, "8080", &hints, &bind_address);
	
	printf("\nCriando o socket...");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, 
		bind_address->ai_socktype, bind_address->ai_protocol);

	//Testando se a chamada da função socket funcionou...
	if (!ISVALIDSOCKET(socket_listen)){
		fprintf(stderr, "socket() falhou... (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("\nVinculando socket ao endereço local (Binding)...");
	if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)){
		fprintf(stderr, "bind() falhou. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);
	
	printf("\nListening...");
	if(listen(socket_listen, 10)<0){
		fprintf(stderr, "listen() falhou. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("\nEsperando por conexão...");
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	SOCKET socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
	if(!ISVALIDSOCKET(socket_client)){
		fprintf(stderr, "accept() falhou. (%d)", GETSOCKETERRNO());
		return 1;
	}

	printf("\nCliente está conectado...");
	char address_buffer[100];
	getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	printf("\n%s", address_buffer);


	printf("\nLendo requisição...");
	char request[1024];
	int bytes_received = recv(socket_client, request, 1024, 0);
	printf("\n%d bytes recebidos...", bytes_received);


	printf("Enviando resposta...");
	const char *response = 
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"Content-Type: text/plain\r\n\r\n"
	"Hora local: ";
	int bytes_sent = send(socket_client, response, strlen(response), 0);
	printf("\nSent %d of %d bytes.", bytes_sent, (int)strlen(response));

	time_t timer;
	time(&timer);
	char *time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);

	printf("\nEncerrando conexão...");
	CLOSESOCKET(socket_client);

	printf("\nEncerrando listening socket");
	CLOSESOCKET(socket_listen);

	printf("\nEncerrado\n");






	return 0;
}
