#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3001
#define BUFFER_SIZE 1024

using namespace std;

void handle_client(int client_sock) { 
    int num1, num2, result; 
    recv(client_sock, &num1, sizeof(num1), 0); 
    recv(client_sock, &num2, sizeof(num2), 0);
    result = num1 + num2;
    cout << "Sending result to client...\n";
    send(client_sock, &result, sizeof(result), 0);
    close(client_sock);
}

void server() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        cerr << "Socket creation failed." << endl;
        return;
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Bind failed." << endl;
        close(server_sock);
        return;
    }

    // Listen for connections
    listen(server_sock, 2);
    cout << "Server listening on port " << PORT << "..." << endl;

    while (true) { 
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock == -1) {
            cerr << "Accept failed." << endl; 
            continue;
        }

        cout << "Client connected!" << endl;
        thread t(handle_client, client_sock);
        t.detach();
    }

    close(server_sock);
}

int main() {
    server();
    return 0;
}
