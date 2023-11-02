#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#pragma(lib, "Ws2_32.lib")

#define HTTP_PORT "80"

int main(){
    struct addrinfo *result = NULL, hints;
    SOCKET ListenSocket = INVALID_SOCKET;
    WSADATA wsaData;
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
    closesocket(ListenSocket);
    WSACleanup();
    return -0;
}