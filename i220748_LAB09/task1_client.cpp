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
    string request = "Send Problem";
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

    // Send request to server
    sendto(sock, request.c_str(), request.length(), 0, (struct sockaddr*)&serv_addr, serv_len);
    
    // Receive problem from server
    int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_len);
    if (recv_len == SOCKET_ERROR) {
        cerr << "Failed to receive problem. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    buffer[recv_len] = '\0';
    cout << "Server: " << buffer << endl;

    // Ask user for answer
    string answer;
    cout << "Your answer: ";
    cin >> answer;

    // Send answer to server
    sendto(sock, answer.c_str(), answer.length(), 0, (struct sockaddr*)&serv_addr, serv_len);

    // Receive response
    memset(buffer, 0, BUFFER_SIZE);
    recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_len);
    if (recv_len == SOCKET_ERROR) {
        cerr << "Failed to receive response. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    buffer[recv_len] = '\0';
    cout << "Server: " << buffer << endl;

    // Cleanup
    closesocket(sock);
    WSACleanup();

    return 0;
}
