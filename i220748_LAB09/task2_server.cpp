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
    SOCKET server_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = { 0 };

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

    // Setup server address
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

    cout << "UDP Server running on port " << PORT << "...\n";

    while (true) {
        // Receive word from client
        memset(buffer, 0, BUFFER_SIZE);
        int recv_len = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len == SOCKET_ERROR) {
            cerr << "Receive failed. Error Code: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[recv_len] = '\0';  // Null-terminate received data
        string word(buffer);
        string response = "The word '" + word + "' has " + to_string(word.length()) + " letters.";

        // Send response to client
        sendto(server_sock, response.c_str(), response.length(), 0, (struct sockaddr*)&client_addr, client_len);

        cout << "Received: " << word << " | Responded: " << response << endl;
    }

    // Cleanup
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
