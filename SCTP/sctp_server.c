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
#define MAXCON 10 //número máximo de conexões que podem ser colocadas em espera enquanto o processo está tratando uma conexão ativa. listen()
#define MAXBUFFER 1024
#define TIMEOUTSEC 0
#define TIMEOUTUSEC 100000
#define CLOSESOCKET(s) close(s)
#define ISVALIDSOCKET(s) ((s) >=0)
#define SOCKET int
#define MYPORT 29168



/*Função que cria um socket para SCTP, IPV4 em modo de fluxo (STREAM) e trata erro*/
SOCKET socket_sctp(){
	SOCKET socket_ret;
    socket_ret = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if(socket_ret== -1){
        printf("\nsocket() falhou");
        exit(-1);
    }
    return socket_ret;
}


/*Função que faz o link entre o socket e o endereço IP:porta do servidor*/
void bind_sctp(SOCKET socket_listen){
    struct sockaddr_in local_address;
    memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(MYPORT);
    if(bind(socket_listen, (struct sockaddr *) &local_address, sizeof(local_address)) <0){
        printf("\nbind() falhou");
        exit(-1);
    }
}


/*Função que aceita novas conexões e imprime o endereço na tela*/
int accept_sctp(SOCKET socket_listen){
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    memset(&client_address, 0, sizeof(client_address));
    SOCKET socket_client = accept(socket_listen, (struct sockaddr *) &client_address, &client_len);
    if(!ISVALIDSOCKET(socket_client)){
        printf("\naccept() falhou");
        return 1;                   
    }
    char address_buffer[100];
    getnameinfo((struct sockaddr *) &client_address, (unsigned int) sizeof(client_address), address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("\nNova conexão de %s\n", address_buffer);
    return socket_client;  
}


/*Função que recebe dados do servidor e retorna o número de bytes recebidos*/
int recv_sctp(SOCKET socket_listen, char *msg){
    int bytes_received = recv(socket_listen, msg, MAXBUFFER, 0);
    return bytes_received;
}


/*Função que envia dados ao servidor e retorna o número de bytes enviados*/
int send_sctp(SOCKET socket_listen){
    char msg[MAXBUFFER];
    printf("\nDigite a mensagem: ");
    if(!fgets(msg, MAXBUFFER, stdin)) return -2;
    int bytes_sent = send(socket_listen, msg, strlen(msg), 0);
    return bytes_sent;
}


int main(){


    /*Criando socket*/
    SOCKET socket_listen = socket_sctp();

    /*Vinculando o socket com IP e porta definidos*/
    bind_sctp(socket_listen);


    /*Esperando por conexão no protocolo SCTP*/
    if(listen(socket_listen, MAXCON)<0){ 
        printf("\nlisten() falhou");
        return 1;
    }


    /*Conjunto dos sockets*/
    fd_set master;
    FD_ZERO(&master);//Limpando conjunto de sockets
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;//Como teremos vários sockets temos sempre que saber o valor do maior para a função select()


    printf("\nEsperando por conexões...\n");

    while(1){
        fd_set reads;
        reads = master;
        if(select(max_socket+1, &reads, 0, 0, 0)<0){ /*Select() serve como uma multiplexação de sockets*/
            printf("\nselect() falhou");
            return 1;
        }
fflush(stdout);
        SOCKET i;
        for(i=1; i<=max_socket; ++i){
            if(FD_ISSET(i, &reads)){
                if( i == socket_listen){
                    SOCKET socket_client = accept_sctp(socket_listen); /*Aceitando novas conexões*/
                    FD_SET(socket_client, &master);
                    if(socket_client>max_socket) /*Atualizando o valor do max_socket*/
                        max_socket= socket_client;
                }else{
                    char msg[MAXBUFFER];       
                    int bytes_received= recv_sctp(i, msg);
                    if(bytes_received<1){
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    printf("\nRecebido: %s\n", msg);

                    /*Processa informação....*/
                    /*Envia o resultado...*/
                }

            }

        }
    }

    /*Fechando o socket*/
    CLOSESOCKET(socket_listen);
    printf("Encerrado...\n");


    return 0;
}

