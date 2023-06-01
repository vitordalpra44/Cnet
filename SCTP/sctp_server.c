/*Programa de server SCTP (Stream Control Transmission Protocol)
  por Vitor Nievola Dalprá                                    */
  



/*Todas as bibliotecas necessárias*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>



/*Todos os defines necessários*/
#define MYPORT "8080"
#define MAXCON 10 /*número máximo de conexões que podem ser colocadas em espera enquanto o processo está tratando uma conexão ativa. listen()*/
#define MAXBUFFER 1024
#define SOCKET int

#define ISVALIDSOCKET(s) ((s) >=0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main(){

/*Configurando endereço local*/
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;

struct addrinfo *bind_address;
if(getaddrinfo(0, MYPORT, &hints, &bind_address)) return 1; /*se getaddrinfo retornar algo diferente de 0 a operação deu errado*/

/*Criando socket*/
SOCKET socket_listen;
socket_listen = socket(bind_address->ai_family,
                        bind_address->ai_socktype, IPPROTO_SCTP);/*Definindo protocolo SCTP*/
                        
if(!ISVALIDSOCKET(socket_listen)){ /*testando se a função socket() funcionou*/
    fprintf(stderr, "socket() falhou (%d)", GETSOCKETERRNO());
    return 1;
}

/*Vinculando o socket com IP e porta definidos*/
if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)){
    fprintf(stderr, "bind() falhou (%d)", GETSOCKETERRNO());
    return 1;
}

freeaddrinfo(bind_address); /*Após o bind ter sido efetuado podemos desalocar*/

/*Esperando por conexão no protocolo SCTP*/
if(listen(socket_listen, MAXCON)<0){ 
    fprintf(stderr, "listen() falhou (%d)", GETSOCKETERRNO());
    return 1;
}

/*Conjunto dos sockets*/
fd_set master;
FD_ZERO(&master);
FD_SET(socket_listen, &master);
SOCKET max_socket = socket_listen;


printf("Esperando por conexões...\n");
while(1){
    fd_set reads;
    reads = master;
    if(select(max_socket+1, &reads, 0, 0, 0)<0){ /*Select() serve como uma multiplexação de sockets*/
        fprintf(stderr, "select() falhou (%d)", GETSOCKETERRNO());
        return 1;
    }
    
    SOCKET i;
    for(i=1; i<=max_socket; ++i){
        if(FD_ISSET(i, &reads)){
            if( i== socket_listen){
                struct sockaddr_storage client_address;
                socklen_t client_len = sizeof(client_address);
                SOCKET socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
                if(!ISVALIDSOCKET(socket_client)){
                    fprintf(stderr, "accept() falhou (%d)", GETSOCKETERRNO());
                    return 1;                   
                }
                FD_SET(socket_client, &master);
                if(socket_client>max_socket)
                    max_socket= socket_client;
                
                char address_buffer[100];
                getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                printf("Nova conexão de %s\n", address_buffer);

            
            }else{
                char read[MAXBUFFER];       
                int bytes_received= recv(i, read, MAXBUFFER, 0);
                if(bytes_received<1){
                    FD_CLR(i, &master);
                    CLOSESOCKET(i);
                    continue;
                }
                printf("Recebido: %s", read);
                /*Processa informação....*/
                /*Envia o resultado...*/
            }

        }

    }

}

printf("Fechando o socket...\n");
CLOSESOCKET(socket_listen);
printf("Encerrado...\n");
    return 0;
}

