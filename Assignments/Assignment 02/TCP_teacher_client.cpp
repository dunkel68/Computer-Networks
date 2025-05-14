#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

const int TCP_PORT = 8888;
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

    cout << "Teacher Client for Record Management System" << endl;
    cout << "===========================================" << endl;

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Error creating socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);

    cout << "Connecting to server at " << SERVER_IP << ":" << TCP_PORT << "..." << endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server successfully." << endl;

    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;
    string userInput;

    // Communication loop
    while (true) {
        // Receiving msg from server
        memset(buffer, 0, MAX_BUFFER_SIZE);
        bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        
        if (bytesReceived <= 0) {
            cout << "Connection closed by server." << endl;
            break;
        }

        string serverMessage(buffer, 0, bytesReceived);
        cout << serverMessage;

        // Check if the server is expecting input
        if (serverMessage.find("Enter your") != string::npos || serverMessage.find("Example:") != string::npos ||
            serverMessage.find("format") != string::npos || serverMessage.find(":") != string::npos) {
            
            getline(cin, userInput);
            
            if (userInput == "exit") {
                cout << "Exiting client application..." << endl;
                break;
            }
            
            // Sending user input to server
            send(clientSocket, userInput.c_str(), userInput.length(), 0);
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    
    cout << "Teacher client application closed." << endl;
    
    return 0;
}