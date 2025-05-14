#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

// Function to generate a random math problem
string generateProblem(int& correctAnswer) {
    int num1 = rand() % 20 + 1;
    int num2 = rand() % 20 + 1;
    char operators[] = { '+', '-', '*' };
    char op = operators[rand() % 3];

    string problem;
    switch (op) {
        case '+': correctAnswer = num1 + num2; problem = to_string(num1) + " + " + to_string(num2); break;
        case '-': correctAnswer = num1 - num2; problem = to_string(num1) + " - " + to_string(num2); break;
        case '*': correctAnswer = num1 * num2; problem = to_string(num1) + " * " + to_string(num2); break;
    }

    return "Solve: " + problem;
}

int main() {
    srand(time(0));  // Seed random generator
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

    cout << "UDP Math Quiz Server running on port " << PORT << "...\n";

    while (true) {
        // Wait for client request
        recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        
        // Generate a new math problem
        int correctAnswer;
        string problem = generateProblem(correctAnswer);

        // Send problem to client
        sendto(server_sock, problem.c_str(), problem.length(), 0, (struct sockaddr*)&client_addr, client_len);
        cout << "Sent to client: " << problem << endl;

        // Receive client's answer
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);

        // Check the answer
        int clientAnswer = atoi(buffer);
        string response = (clientAnswer == correctAnswer) ? "Correct!" : "Incorrect!";

        // Send the result
        sendto(server_sock, response.c_str(), response.length(), 0, (struct sockaddr*)&client_addr, client_len);
        cout << "Client Answer: " << clientAnswer << " | Expected: " << correctAnswer << " | " << response << endl;
    }

    // Cleanup
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
