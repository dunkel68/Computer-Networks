#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

std::vector<int> clients;
std::mutex clients_mutex;

void handleClient(int client_sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Client disconnected\n";
            break;
        }

        std::string message(buffer);
        std::cout << "Client " << client_sock << ": " << message << std::endl;

        if (message == "bye") {
            break;
        }

        // Server's response
        std::string response;
        std::cout << "Server: ";
        std::getline(std::cin, response);

        send(client_sock, response.c_str(), response.size(), 0);
    }

    // Remove client from list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_sock), clients.end());
    }

    close(client_sock);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // Listen for clients
    if (listen(server_sock, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Client accept failed\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_sock);
        }

        std::cout << "New client connected: " << std::endl;
        std::thread(handleClient, client_sock).detach();
    }

    close(server_sock);
    return 0;
}
