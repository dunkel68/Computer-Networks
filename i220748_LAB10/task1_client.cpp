#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
using namespace std;

string solve_task(const string& task) {
    if (task.find("Reverse this string:") != string::npos) {
        string word = task.substr(task.find(":") + 2);  // Extract word dynamically
        reverse(word.begin(), word.end());
        return word;
    } 

    if (task.find("Sum these numbers:") != string::npos) {
        stringstream ss(task.substr(task.find(":") + 2));
        int num, sum = 0;
        while (ss >> num) {
            sum += num;
            if (ss.peek() == ',' || ss.peek() == ' ') ss.ignore();
        }
        return to_string(sum);
    }

    return "Invalid Task";
}


int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed." << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;
    int correct_answers = 0;

    while (correct_answers < 2) {
        int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;

        buffer[bytesReceived] = '\0';
        cout << "Server: " << buffer << endl;

        string answer = solve_task(buffer);
        send(sock, answer.c_str(), answer.length(), 0);

        bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Server Response: " << buffer << endl;
            if (string(buffer) == "Accepted!") correct_answers++;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}