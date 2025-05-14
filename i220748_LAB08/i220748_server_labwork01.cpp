#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1000] = {0};
    std::string message = "Hi";

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Bind failed.\n";
        close(server_sock);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_sock, 3) == -1) {
        std::cerr << "Listen failed.\n";
        close(server_sock);
        return 1;
    }
    std::cout << "Server listening on port 8080 ...\n";

    // Accept client connection
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock == -1) {
        std::cerr << "Client accept failed.\n";
        close(server_sock);
        return 1;
    }
    std::cout << "Client connected!\n";

    send(client_sock, message.c_str(), message.length(), 0);
    std::cout << "Server: " << message << std::endl;

    recv(client_sock, buffer, 1000, 0);
    std::cout << "Client: " << buffer << std::endl;

    close(client_sock);
    close(server_sock);

    return 0;
}
