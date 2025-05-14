#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
using namespace std;

int main() {
    WSADATA wsa;
    SOCKET server_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = { 0 };
    string message = "Hi";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }

    // Create UDP socket
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    cout << "UDP Server listening on port " << PORT << "..." << endl;

    // Receive message from client
    recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
    cout << "Client: " << buffer << endl;

    // Send response to client
    sendto(server_sock, message.c_str(), message.length(), 0, (struct sockaddr*)&client_addr, client_len);
    cout << "Server: Hi" << endl;

    // Close socket
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
