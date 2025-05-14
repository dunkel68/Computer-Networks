#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MSG_COUNT 5

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return 1;
    }

    // Sending 5 messages
    for (int i = 1; i <= MSG_COUNT; i++) {
        std::string msg = "Message " + std::to_string(i);
        send(sock, msg.c_str(), msg.length(), 0);
        std::cout << "Client: " << msg << std::endl;
        sleep(1);
    }

    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    std::cout << "Server: " << buffer << std::endl;

    close(sock);

    return 0;
}
