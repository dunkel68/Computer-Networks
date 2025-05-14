#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

const int UDP_PORT = 8889;
const int MAX_BUFFER_SIZE = 1024;
const string SERVER_IP = "127.0.0.1";

bool initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return false;
    }
    return true;
}

int main() {
    if (!initWinsock()) {
        return 1;
    }

    cout << "Student Client for Record Management System" << endl;
    cout << "===========================================" << endl;

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Error creating socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);

    string studentId;
    char buffer[MAX_BUFFER_SIZE];
    int serverAddrSize = sizeof(serverAddr);
    bool running = true;

    while (running) {
        cout << "\nEnter your Student ID (or 'exit' to quit): ";
        getline(cin, studentId);
        
        if (studentId == "exit") {
            running = false;
            continue;
        }
        
        // Sending to server
        cout << "Sending request for student ID: " << studentId << endl;
        sendto(clientSocket, studentId.c_str(), studentId.length(), 0, 
               (sockaddr*)&serverAddr, serverAddrSize);
        
        // Receiving response from server
        memset(buffer, 0, MAX_BUFFER_SIZE);
        int bytesReceived = recvfrom(clientSocket, buffer, MAX_BUFFER_SIZE, 0, 
                                    (sockaddr*)&serverAddr, &serverAddrSize);
        
        if (bytesReceived == SOCKET_ERROR) {
            cerr << "Error receiving data: " << WSAGetLastError() << endl;
            continue;
        }
        
        string serverResponse(buffer, 0, bytesReceived);
        cout << "\n=========== STUDENT RECORD ===========\n" << endl;
        cout << serverResponse << endl;
        cout << "\n=======================================" << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    
    cout << "Student client application closed." << endl;
    
    return 0;
}