#include "cap6.h"
#define TIMEOUT 5.0

void parse_url(char *url, char **hostname, char **port, char **path);
void send_request(SOCKET s, char *hostname, char *port, char *path);
SOCKET connect_to_host(char *hostname, char *port);

int main(int argc, char *argv[]){
    if (argc<2){
        fprintf(stderr, "usage: web_get url\n");
        return 1;
    }

    char *url = argv[1];
    char *hostname, *port, *path;
    parse_url(url, &hostname, &port, &path);

    SOCKET server = connect_to_host(hostname, port);
    send_request(server, hostname, port, path);
    const clock_t start_time = clock();
    
    #define RESPONSE_SIZE 8192
        char response[RESPONSE_SIZE+1];
        char *p = response, *q;
        char *end = response + RESPONSE_SIZE;
        char *body = 0;

        enum {length, chunked, connection};
        int encoding = 0;
        int remaining = 0;
        






}

void parse_url(char *url, char **hostname, char **port, char **path){
    printf("URL: %s\n", url);
    char *p;
    p = strstr(url, "://");
    
    char *protocol = 0;
    if(p){
        protocol = url;
        *p = 0;
        p += 3;
    }else {
        P = url;
    }
    if(protocol){
        if(strcmp(protocol, "http")){
            fprintf(stderr, "Protocolo desconhecido '%s'. Só protocolo 'http' é suportado.", protocol);
            exit(1);
        }
    }

    *hostname = p;
    while(*p && *p != ':' && *p!='/' && *p!='#') ++p;
    *path=p;
    if(*p == '/'){
        *path = p+1;
    }
    *p = 0;

    while(*p && *p!='#') ++p;
    if(*p == '#') *p = 0;

    printf("hostname: %s\n", *hostname);
    printf("porta: %s\n", *port);
    printf("caminho: %s\n", *path);
}

void send_request(SOCKET s, char *hostname, char *port, char *path){
    char buffer[2048];
    sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
    sprintf(buffer + strlen(buffer), "\r\n");

    send(s, buffer, strlen(buffer), 0);
    printf("Sent Headers: \n%s", buffer);
}

SOCKET connect_to_host(char *hostname, char *port){
    printf("Configurando endereço remoto...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if(getaddrinfo(hostname, port, &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() falhou. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    printf("Endereço remoto é: ");
    char adress_buffer[100];
    char service_buffer[100];

    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, adress_buffer, sizeof(adress_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    printf("Criando o socket...\n");
    SOCKET server;
    server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(server)){
        fprintf(stderr, "socket() falhou. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    printf("Conectando...\n");
    if(connect(server, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "connect() falhou. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    freeaddrinfo(peer_address);
    printf("Conectado.\n\n");
    return server;
}