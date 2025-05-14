#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
#define INITIAL_CWND 1 // Initial congestion window
#define MSS 512 // Maximum Segment Size
#define MAX_PACKETS 20 // Limit to 20 packets
using namespace std;
int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = { 0 };
    int cwnd = INITIAL_CWND;
    int ssthresh = 8; // Slow-start threshold
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }
    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    // Set server address
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
    if (listen(server_sock, 3) == SOCKET_ERROR) {
        cerr << "Listen failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    cout << "Server listening on port " << PORT << "..." << endl;
    // Accept client connection
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock == INVALID_SOCKET) {
        cerr << "Client accept failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    cout << "Client connected!" << endl;
    int packetCount = 0;
    while (packetCount < MAX_PACKETS) {
        int bytesToSend = min(cwnd * MSS, BUFFER_SIZE);
        string message = "Data Packet (" + to_string(packetCount + 1) + "), cwnd=" + to_string(cwnd);
        send(client_sock, message.c_str(), bytesToSend, 0);
        cout << "Sent: " << message << endl;
        // Wait for ACK
        int bytesReceived = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            cout << "Client disconnected or error occurred." << endl;
            break;
        }
        buffer[bytesReceived] = '\0';
        cout << "Received: " << buffer << endl;
        // Simulate congestion control behavior
        if (rand() % 10 < 1) { // Simulate 10% packet loss
            cout << "Packet loss detected! Multiplicative decrease." << endl;
            ssthresh = max(2, cwnd / 2);
            cwnd = 1;
        }
        else {
            if (cwnd < ssthresh) {
            cwnd *= 2; // Slow start (exponential growth)
            }
            else {
            cwnd += 1; // Congestion avoidance (linear growth)
            }
        }
        packetCount++;
        this_thread::sleep_for(chrono::milliseconds(500)); // Simulate RTT delay
    }
    cout << "Server: Completed 20 packets. Closing connection." << endl;
    // Close sockets
    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}