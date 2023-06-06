/*Programa de client SCTP (Stream Control Transmission Protocol)
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
#define MAXBUFFER 1024
#define TIMEOUTSEC 0
#define TIMEOUTUSEC 100000
#define CLOSESOCKET(s) close(s)
#define SOCKET int


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


/*Função que recebe IP, porta e um socket ativo e o conecta ao servidor em questão*/
int connect_sctp(char *IP, char *PORT, SOCKET socket_peer){
    struct sockaddr_in remote_peer;
    memset(&remote_peer, 0, sizeof(remote_peer));
    remote_peer.sin_family = AF_INET;
    remote_peer.sin_addr.s_addr = inet_addr(IP);
    remote_peer.sin_port = htons(atoi(PORT));
    if(connect(socket_peer, (struct sockaddr *) &remote_peer, sizeof(remote_peer)) < 0){
        printf("\nconnect() falhou");
        return -1;
    }
    printf("\nConectado...\n");
    fflush(stdout);
    return 0;
}


/*Função que recebe dados do servidor e retorna o número de bytes recebidos*/
int recv_sctp(SOCKET socket_peer, char *msg){
    int bytes_received = recv(socket_peer, msg, MAXBUFFER, 0);
    if (bytes_received<1){
        printf("\nconexão interrompida pelo peer\n");
        return -1;
    }

}


/*Função que envia dados ao servidor e retorna o número de bytes enviados*/
int send_sctp(SOCKET socket_peer){
    char msg[MAXBUFFER];
    printf("\nDigite a mensagem: ");
    if(!fgets(msg, MAXBUFFER, stdin)) return -2;
    int bytes_sent = send(socket_peer, msg, strlen(msg), 0);
    return bytes_sent;
}


int main(int argc, char *argv[]){


    /*Caso os parâmetros da main não tenham sido passados de forma correta*/
    if(argc<3){
        fprintf(stderr, "Formato: ./tcp_client IP1 port\n");
        return 1;
    }


    printf("\nConfigurando o endereço remoto...\n");


    /*Criando o socket*/
    SOCKET socket_peer = socket_sctp();


    /*Conectando ao servidor*/
    if(connect_sctp(argv[1], argv[2], socket_peer)) return -1;


    printf("Teste");
    fflush(stdout);
    while(1){


        /*Esse bloco cria o conjunto de sockets e chama a função select() para monitorar*/
        fd_set reads;
        FD_ZERO(&reads); //Limpando conjunto de sockets
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);//Descritor padrão de escrita (stdin)
        struct timeval timeout;
        timeout.tv_sec = TIMEOUTSEC;
        timeout.tv_usec = TIMEOUTUSEC;
        if(select(socket_peer+1, &reads, 0, 0, &timeout)<0){
            fprintf(stderr, "select() falhou");
            return 1;
        }


        if(FD_ISSET(socket_peer, &reads)){
            /*Recebendo as informações do servidor*/
            char msg[MAXBUFFER];
            int bytes_received = recv_sctp(socket_peer, msg);
            if (bytes_received<1) break;
            printf("\nRecebido: %.*s\n",bytes_received, msg);
        }


        if(FD_ISSET(0, &reads)){
            /*Enviando as informações para o servidor*/
            int bytes_sent = send_sctp(socket_peer);
            if (bytes_sent == -2) break; //Condição em que fgets não recebeu nada
        }


    }


    /*Fechando socket*/
    CLOSESOCKET(socket_peer);
    printf("\nEncerrado...\n");


    return 0;
}