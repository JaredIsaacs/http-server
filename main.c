#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#pragma(lib, "Ws2_32.lib")

int main(){
    WSADATA wsaData;
    int r;

    // Initialize Winsock
    r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(r != 0){
        printf("WSAStartup failed: %d\n", r);
        return -1;
    }

    return -0;
}