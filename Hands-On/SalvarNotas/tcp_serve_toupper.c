#include "SalvarNotasH.h"
#include <ctype.h>
#define PASSWORD "senha123\n"
#define MAXSOCK 10
int main(){

 int logado[MAXSOCK] = {0};




//Configurando endereço local;
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;

struct addrinfo *bind_address;
getaddrinfo(0, "8080", &hints, &bind_address);

//Criando socket
SOCKET socket_listen;
socket_listen = socket(bind_address->ai_family,
                        bind_address->ai_socktype, bind_address->ai_protocol);
if(!ISVALIDSOCKET(socket_listen)){
    fprintf(stderr, "Socket falhou (%d)", GETSOCKETERRNO());
    return 1;
}

//Vinculando o socket com o endereço local
if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)){
    fprintf(stderr, "Bind falhou (%d)", GETSOCKETERRNO());
    return 1;
}

freeaddrinfo(bind_address);

//Esperando por conexão
if(listen(socket_listen, 10)<0){
    fprintf(stderr, "Listen falhou (%d)", GETSOCKETERRNO());
    return 1;
}

fd_set master;
FD_ZERO(&master);
FD_SET(socket_listen, &master);
SOCKET max_socket = socket_listen;

printf("Esperando por conexões...\n");
while(1){
    fd_set reads;
    reads = master;
    if(select(max_socket+1, &reads, 0, 0, 0)<0){
        fprintf(stderr, "Select falhou (%d)", GETSOCKETERRNO());
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
                    fprintf(stderr, "accept falhou (%d)", GETSOCKETERRNO());
                    return 1;                   
                }
                FD_SET(socket_client, &master);
                if(socket_client>max_socket)
                    max_socket= socket_client;
                
                char address_buffer[100];
                getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                printf("Nova conexão de %s\n", address_buffer);
            
            }else{
                if(logado[i] == 0){
                    unsigned char read[1024];
                    for(int j=0;j<1024;j++) read[j]=0;
                    int bytes_received= recv(i, read, 1024, 0);
                    if(bytes_received<1){
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    
                    if(!strcmp(read, PASSWORD)){
                        logado[i] = 1;
                        unsigned char mensagem2[] = {"Digite sua anotação: "};
                        send(i, mensagem2, strlen(mensagem2), 0);
                    }else{
                        logado[i] = 0;
                        unsigned char mensagem2[] = {"Digite a senha: "};
                        send(i, mensagem2, strlen(mensagem2), 0);
                    }
                }else if(logado[i] == 1){
                    unsigned char read[1024];
                    for(int j=0;j<1024;j++) read[j]=0;
                    int bytes_received= recv(i, read, 1024, 0);
                    if(bytes_received<1){
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    FILE *arquivo;
                    arquivo = fopen("Notas.txt", "a");
                    if(arquivo == NULL) {
                        printf("Erro ao abrir o arquivo!\n");
                        return 1;
                    }

                    time_t timer;
                    time(&timer);
                    fprintf(arquivo, "Data: %sNota: %s", ctime(&timer), read);
                    fclose(arquivo);
                    for(int j=0;j<1024;j++){
                        read[j]=0;
                    }
                    char resp[] = {"\nNota salva com sucesso..."};
                    send(i, resp, strlen(resp),0);
                    logado[i]=0;
                    FD_CLR(i, &master);
                    CLOSESOCKET(i);
                }
            }
        }

    }

}

printf("Fechando o socket...\n");
CLOSESOCKET(socket_listen);
printf("Encerrado...\n");





    return 0;
}