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
#define MAXCON 10 /*número máximo de conexões que podem ser colocadas em espera enquanto o processo está tratando uma conexão ativa. listen()*/
#define MAXBUFFER 1024
#define TIMEOUTSEC 0
#define TIMEOUTUSEC 100000
#define SOCKET int

#define ISVALIDSOCKET(s) ((s) >=0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

struct sockaddr_storage s_rem, s_loc;
struct sockaddr_in address;
int socket_r(char *IP, char *port){ //Função robusta que cria um socket para SCTP e trata erros *Tirada da biblioteca lksctp e adaptada
	struct sctp_event_subscribe subscribe;
	int sk, error;
    
    // Configurar o endereço IPv4
    address.sin_family = AF_INET;  // Família de endereços IPv4
    address.sin_port = htons(port);  // Porta
    inet_pton(AF_INET, IP, &(address.sin_addr));  // Endereço IP

    // Copiar o endereço IPv4 para a estrutura sockaddr_storage
    memcpy(&s_loc, &address, sizeof(struct sockaddr_in));
	if ((sk = socket(s_loc.ss_family, SOCK_STREAM, IPPROTO_SCTP)) < 0 ) {
			return -1;
	}

	memset(&subscribe, 0, sizeof(subscribe));
	subscribe.sctp_data_io_event = 1;
	subscribe.sctp_association_event = 1;
	error = setsockopt(sk, SOL_SCTP, SCTP_EVENTS, (char *)&subscribe,
			   sizeof(subscribe));
	if (error) {
		fprintf(stderr, "SCTP_EVENTS: error: %d\n", error);
		exit(1);
	}
	return sk;

}

int main(int argc, char *argv[]){


    /*Caso os parâmetros da main não tenham sido passados de forma correta*/
    if(argc<3){
        fprintf(stderr, "Formato: ./tcp_client IP1 port\n");
        return 1;
    }

    printf("\nConfigurando o endereço remoto...\n");

    /*Criando o socket*/
    SOCKET socket_peer = socket_r(argv[1], argv[2]); /*Definindo protocolo SCTP*/

    /*Conectando ao servidor*/
    if(connect(socket_peer, s_loc->ai_addr, s_loc->ai_addrlen)){
        fprintf(stderr, "connect() falhou. (%d)", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);
    printf("Conectado...");
    
    


    while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);
        struct timeval timeout;
        timeout.tv_sec = TIMEOUTSEC;
        timeout.tv_usec = TIMEOUTUSEC;
        if(select(socket_peer+1, &reads, 0, 0, &timeout)<0){
            fprintf(stderr, "select() falhou. (%d)", GETSOCKETERRNO());
            return 1;
        }

        if(FD_ISSET(socket_peer, &reads)){
            char read[MAXBUFFER];
            int bytes_received = recv(socket_peer, read, MAXBUFFER, 0);
        
            if (bytes_received<1){
                printf("Conexão interrompida pelo peer\n");
                break;
            }
            printf("\nRecebido: %.*s\n",bytes_received, read);

        }

        if(FD_ISSET(0, &reads)){
            /*Enviando as informações para o servidor*/
            char read[MAXBUFFER];
            printf("\nDigite a mensagem: ");
            if(!fgets(read, MAXBUFFER, stdin)) break;
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
        }
    }

    /*Fechando socket*/
    CLOSESOCKET(socket_peer);
    printf("\nEncerrado\n");
    return 0;
}