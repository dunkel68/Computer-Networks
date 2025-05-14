#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    std::string response = "Hello";

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    // Set server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address using inet_pton()
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        close(sock);
        return 1;
    }

    recv(sock, buffer, BUFFER_SIZE, 0);
    std::cout << "Server: " << buffer << std::endl;

    send(sock, response.c_str(), response.length(), 0);
    std::cout << "Client: " << response << std::endl;

    close(sock);

    return 0;
}
