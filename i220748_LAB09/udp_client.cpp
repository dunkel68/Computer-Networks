#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = { 0 };
    string message = "Hello";
    int serv_len = sizeof(serv_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Set up server address
    memset(&serv_addr, 0, sizeof(serv_addr));  // Clear struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        cerr << "Invalid address / Address not supported" << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Send message to server
    if (sendto(sock, message.c_str(), message.length(), 0, (struct sockaddr*)&serv_addr, serv_len) == SOCKET_ERROR) {
        cerr << "Failed to send message. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    cout << "Client: " << message << endl;

    // Receive response from server
    int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_len);
    if (recv_len == SOCKET_ERROR) {
        cerr << "Failed to receive message. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    buffer[recv_len] = '\0';  // Ensure null-termination
    cout << "Server: " << buffer << endl;

    // Close socket
    closesocket(sock);
    WSACleanup();
    
    return 0;
}
