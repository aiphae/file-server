#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char **argv) {
    WSADATA wsa_data;
    int result;

    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("WSAStartup() failed: %d\n", result);
        return 1;
    }

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.S_un.S_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(listen_socket, (SOCKADDR *) &address, sizeof(address)) == SOCKET_ERROR) {
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    printf("Waiting for a connection...\n");

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    SOCKET client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        printf("accept() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    int buffer_size = 512;
    char buffer[buffer_size];

    int bytes_received = recv(client_socket, buffer, buffer_size, 0);
    if (bytes_received > 0) {
        printf("Received: %.*s\n", bytes_received, buffer);
        const char *response = "Hello from server!";
        send(client_socket, response, (int) strlen(response), 0);
    }

    closesocket(client_socket);
    closesocket(listen_socket);
    WSACleanup();

    return 0;
}
