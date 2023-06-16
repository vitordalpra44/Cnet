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
        fflush(stdout);
        return -1;
    }

}
FILE * abrirArquivo(const char *path)
{
    FILE *fv;
    if (!(fv = fopen(path, "rb"))) {
	printf("Failed to open the '%s' file. Restart the sample program using the file "
		"location as the input parameter.\n", path);
        fflush(stdout);
        exit(1);
    }
    return fv;
}
FILE * abrirArquivoHex(const char *path)
{
    FILE *fv;
    if (!(fv = fopen(path, "r"))) {
	printf("Failed to open the '%s' file. Restart the sample program using the file "
		"location as the input parameter.\n", path);
        fflush(stdout);
        exit(1);
    }
    return fv;
}
/*Função para pegar os bits de um arquivo .per do asn1*/
long  lerArquivo(char *mensagem, const char *path){
    FILE *file = abrirArquivo(path);

    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);

    mensagem = (char*) calloc(tamanho,sizeof(char));
    int resultado = fread(mensagem, sizeof(char), tamanho, file);
    if(file)
    fclose(file);

    if(resultado==tamanho) return tamanho;
    exit(1);
//Lembrar de desalocar a mensagem...
}
/*Função para pegar os bits de um arquivo em hexadecimal do asn1*/
long  lerArquivoHex(unsigned char **mensagem, const char *path){
    FILE *file = abrirArquivoHex(path);

    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *mensagemHex = (unsigned char*) calloc(tamanho,sizeof(unsigned char));
    long resultado = fread(mensagemHex, sizeof(char), tamanho, file);
    if(resultado == tamanho){
        size_t byte_len = tamanho / 2;
        *mensagem = (unsigned char*) malloc(byte_len);
        if(*mensagem == NULL)
            exit(1);
        memset(*mensagem, 0, byte_len);
        for (size_t i = 0; i < byte_len; i++) {
            sscanf(mensagemHex + (2 * i), "%2hhx", *mensagem+i);
        }
        free(mensagemHex);
    }
    if(file)
        fclose(file);
    if(resultado==tamanho) return tamanho/2;
    exit(1);
//Lembrar de desalocar a mensagem...
}
/*Função que envia dados ao servidor e retorna o número de bytes enviados*/
int send_sctp(SOCKET socket_peer){
    int bytes_sent=-1;
    char msg[MAXBUFFER];


    /*printf("\nDigite a mensagem: ");
    if(!fgets(msg, MAXBUFFER, stdin)) return -2;
    int bytes_sent = send(socket_peer, msg, strlen(msg), 0);
    return bytes_sent;*/


    if(!fgets(msg, MAXBUFFER, stdin)) return -2;
    unsigned char *mensagem = NULL;
    int tamanho = lerArquivoHex(&mensagem, "/home/vitor/Desktop/Hugtak/Cnet/SCTP/Teste.txt");
    if(mensagem){
    bytes_sent = send(socket_peer, mensagem, tamanho, 0);
    free(mensagem);
    }

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

    printf("\nEnter para enviar Write-Replace-Warning Request: ");
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
            //printf("\nRecebido: %.*s\n",bytes_received, msg);
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