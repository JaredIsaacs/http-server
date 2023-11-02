#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <pthread.h>

#pragma(lib, "Ws2_32.lib")

#define HTTP_PORT "80"
#define BUFLEN 1024

void* handle_client(void *args);

void get_requested_file(char *recvbuf, char *file_name);

void create_http_response();

int main(){
    struct addrinfo *result = NULL, hints;
    SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
    WSADATA wsaData;
    char recvbuf[BUFLEN];
    int recvbuflen = BUFLEN;
    int r;

    // Initialize Winsock
    r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(r != 0){
        printf("WSAStartup failed: %d\n", r);
        return 1;
    }

    // Resolve the local address ad prot to be used by the server
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    r = getaddrinfo(NULL, HTTP_PORT, &hints, &result);
    if(r != 0){
        printf("getaddrinfo failed: %d\n", r);
        WSACleanup();
        return 1;
    }

    // Create a socket for the server to listen for client connections
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(ListenSocket == INVALID_SOCKET){
        printf("socket failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    r = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if(r == SOCKET_ERROR){
        printf("bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // Setup listening on ListenSocket
    r = listen(ListenSocket, SOMAXCONN);
    if(r == SOCKET_ERROR){
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // loop that accepts client sockets
    while(1){
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if(ClientSocket == INVALID_SOCKET){
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }

        pthread_t t_id;
        pthread_create(&t_id, NULL, handle_client, (void *)&ClientSocket);
        pthread_detach(t_id);
    }

    puts("Shutting down...");

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}

void* handle_client(void *arg){
    SOCKET ClientSocket = *((SOCKET *) arg);
    char recvbuf[BUFLEN], *sendbuf = 
                            "HTTP/1.1 200 OK\r\n"
                            "Connection: close\r\n"
                            "Content-Type: text/html\r\n\r\n"
                            "<h1> Server test </h1>";
    char file_name[64] = "";
    int recvbuflen = BUFLEN;
    int r, sendr;

    r = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if(r > 0 && strstr(recvbuf, "GET")){
        printf("----------\nBytes received: %d\n----------\n%s", r, recvbuf);
        get_requested_file(recvbuf, file_name);
        sendr = send(ClientSocket, sendbuf, strlen(sendbuf), 0);
        if (sendr == SOCKET_ERROR){
            printf("send failed: %d\n", WSAGetLastError());
        }
        printf("Bytes sent: %d\n", sendr);
    }else if(r < 0){
        printf("recv failed: %d\n", WSAGetLastError());
    }

    closesocket(ClientSocket);
    return NULL;
}

void get_requested_file(char *recvbuf, char *file_name){
    char *temp;
    int size;

    temp = strchr(recvbuf, '/');
    size = strcspn(temp, " ");
    strncat(file_name, temp, size);
}