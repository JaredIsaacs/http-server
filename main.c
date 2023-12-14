#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <winsock2.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#pragma(lib, "Ws2_32.lib")

#define HTTP_PORT "80"
#define BUFLEN 1024

void* handle_client(void *args);

char* get_file_name(char *recvbuf);

char* get_file_extension(char *file_name);

void build_response();

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
    char *file_name;
    int recvbuflen = BUFLEN;
    int r, sendr;

    r = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if(r > 0 && strstr(recvbuf, "GET")){
        //Get File name
        file_name = get_file_name(recvbuf);
        sendr = send(ClientSocket, sendbuf, strlen(sendbuf), 0);

        //Get file extension  
        char file_ext[32];
        strcpy(file_ext, get_file_extension(file_name));

        if (sendr == SOCKET_ERROR){
            printf("send failed: %d\n", WSAGetLastError());
        }
        printf("Bytes sent: %d\n", sendr);
    }else if(r < 0){
        printf("recv failed: %d\n", WSAGetLastError());
    }
    
    free(file_name);
    closesocket(ClientSocket);
    return NULL;
}

char* get_file_name(char* recvbuf) {
    char* temp;
    char* file_name;
    int size;

    temp = strchr(recvbuf, '/');
    if (temp == NULL) {
        return NULL;
    }
    
    size = strcspn(temp, " ");

    file_name = malloc(size + 1);
    if (file_name == NULL) {
        return NULL;
    }

    strncpy(file_name, temp, size);
    file_name[size] = '\0';

    return file_name;
}

char* get_file_extension(char *file_name){
  char* ext = strrchr(file_name, '.');
  if(!ext || ext == file_name){
    return "";
  }
  return ext + 1;
}
