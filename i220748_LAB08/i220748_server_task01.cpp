#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MSG_COUNT 5

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_sock);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_sock, 3) < 0) {
        std::cerr << "Listen failed\n";
        close(server_sock);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    // Accept client connection
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        std::cerr << "Client accept failed\n";
        close(server_sock);
        return 1;
    }

    std::cout << "Client connected!\n";

    // Receiving messages from client
    for (int i = 0; i < MSG_COUNT; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);
        std::cout << "Client: " << buffer << std::endl;
    }

    std::string done_msg = "Done!";
    send(client_sock, done_msg.c_str(), done_msg.length(), 0);
    
    close(client_sock);
    close(server_sock);

    return 0;
}
