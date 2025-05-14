#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
using namespace std;
int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = { 0 };
    string ack = "ACK";
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    // Set server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port =htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address/Address not supported" << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock); WSACleanup();
        return 1;
    }
    cout << "Connected to server." << endl;
    int packetCount = 0;
    while (packetCount < 20) {
        int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            cout << "Server closed connection or error occurred." << endl;
            break;
        }
        buffer[bytesReceived] = '\0';
        cout << "Server: " << buffer << endl;
        // Send ACK
        send(sock, ack.c_str(), ack.length(), 0);
        packetCount++;
        this_thread::sleep_for(chrono::milliseconds(100)); // Simulate network delay 
    }
    cout << "Client: Completed 20 packets. Closing connection." << endl;
    // Close socket
    closesocket(sock);
    WSACleanup();
    return 0;
}
