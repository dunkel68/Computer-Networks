#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 20
#define INITIAL_CWND 1

using namespace std;

int cwnd = INITIAL_CWND;  // Congestion window
int active_clients = 0;
mutex mtx;

struct Task {
    string question;
    string answer;
};

// Sample programming tasks
vector<Task> tasks = {
    {"Reverse this string: hello", "olleh"},
    {"Sum these numbers: 1,2,3,4", "10"},
};

void handle_client(SOCKET client_sock, int client_id) {
    int correct_answers = 0;
    auto start_time = chrono::steady_clock::now();
    
    for (int i = 0; i < tasks.size(); i++) {
        string task = tasks[i].question;
        send(client_sock, task.c_str(), task.length(), 0);
        cout << "Sent task to Client #" << client_id << ": " << task << endl;

        char buffer[BUFFER_SIZE] = {0};
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_sock, &readfds);

        timeval timeout;
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        int activity = select(0, &readfds, NULL, NULL, &timeout);
        if (activity <= 0) {
            cout << "Client #" << client_id << " timed out!" << endl;
            closesocket(client_sock);

            lock_guard<mutex> lock(mtx);
            active_clients--;
            cwnd = max(1, cwnd / 2);  // Halve cwnd on timeout
            return;
        }

        int bytesReceived = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string response(buffer);

            if (response == tasks[i].answer) {
                correct_answers++;
                string reply = "Accepted!";
                send(client_sock, reply.c_str(), reply.length(), 0);
            } else {
                string reply = "Wrong Answer!";
                send(client_sock, reply.c_str(), reply.length(), 0);
            }
        }
    }

    if (correct_answers == 3) {
        string win_msg = "Congratulations! You solved all tasks.";
        send(client_sock, win_msg.c_str(), win_msg.length(), 0);

        lock_guard<mutex> lock(mtx);
        cwnd++;  // Increase cwnd when a client wins
    }

    cout << "Client #" << client_id << " session complete. Closing connection." << endl;
    
    lock_guard<mutex> lock(mtx);
    active_clients--;
    
    closesocket(client_sock);
}

int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    cout << "Server listening on port " << PORT << "..." << endl;

    while (true) {
        lock_guard<mutex> lock(mtx);
        if (active_clients >= cwnd) {
            cout << "Congestion: Max clients reached. Waiting..." << endl;
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        }

        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock == INVALID_SOCKET) continue;

        active_clients++;
        thread(handle_client, client_sock, active_clients).detach();
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}

