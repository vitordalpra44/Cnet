#include "SalvarNotasH.h"

int main(int argc, char *argv[]){

    if(argc<3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    printf("\nConfigurando o endereço remoto...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;


    struct addrinfo *peer_address;
    if(getaddrinfo(argv[1], argv[2], &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    /*printf("Endereço remoto é: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer),
    service_buffer, sizeof(service_buffer), NI_NUMERICHOST);

    printf("%s %s\n", address_buffer, service_buffer);*/

    //Criando o socket
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                        peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr,"Socket falhou. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    //Conectando
    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "Conexão falhou. (%d)", GETSOCKETERRNO());
        return 1;
    }
    

    freeaddrinfo(peer_address);
    
    system("echo Pressione enter...");

    while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if(select(socket_peer+1, &reads, 0, 0, &timeout)<0){
            fprintf(stderr, "select() falhou. (%d)", GETSOCKETERRNO());
            return 1;
        }

        if(FD_ISSET(socket_peer, &reads)){
            char read2[4096];
            int bytes_received = recv(socket_peer, read2, 4096, 0);
        
            if (bytes_received<1){
                //printf("Conexão interrompida pelo peer\n");
                break;
            }
            printf("\n%.*s\n",bytes_received, read2);

        }

        if(FD_ISSET(0, &reads)){
            unsigned char read[4096];
            for(int j=0;j<4096;j++) read[j]=0;
            if(!fgets(read, 4096, stdin)) break;
            __fpurge(stdin);
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            system("clear");
        }
    }

    //Fechando socket
    CLOSESOCKET(socket_peer);
    printf("\nEncerrado");
    return 0;
}