#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = { 0 };
    string message = "Hi";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }

    // Create TCP socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Enable address reuse
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) { 
        cerr << "Bind failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    cout << "TCP Server listening on port " << PORT << "..." << endl;

    // Accept incoming connection
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock == INVALID_SOCKET) {
        cerr << "Accept failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    cout << "Client connected!" << endl;

    // Receive message from client
    int recv_len = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (recv_len == SOCKET_ERROR) {
        cerr << "Receive failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(client_sock);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    buffer[recv_len] = '\0';  // Null-terminate the received message
    cout << "Client: " << buffer << endl;

    // Send response to client
    if (send(client_sock, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
        cerr << "Send failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(client_sock);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    cout << "Server: " << message << endl;

    // Close client socket
    closesocket(client_sock);

    // Close server socket
    closesocket(server_sock);
    WSACleanup();

    return 0;
}
