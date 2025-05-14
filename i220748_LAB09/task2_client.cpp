#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = { 0 };
    string word;
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

    // Setup server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        cerr << "Invalid server address" << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    while (true) {
        // Get word input
        cout << "Enter a word (or 'exit' to quit): ";
        cin >> word;
        if (word == "exit") break;

        // Send word to server
        sendto(sock, word.c_str(), word.length(), 0, (struct sockaddr*)&serv_addr, serv_len);

        // Receive response from server
        memset(buffer, 0, BUFFER_SIZE);
        int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_len);
        if (recv_len == SOCKET_ERROR) {
            cerr << "Failed to receive response. Error Code: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[recv_len] = '\0';
        cout << "Server: " << buffer << endl;
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();

    return 0;
}
