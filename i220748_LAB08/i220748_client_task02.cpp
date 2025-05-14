#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveMessages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Disconnected from server\n";
            break;
        }
        std::cout << "\nServer: " << buffer << std::endl;
        std::cout << "You: ";
        std::cout.flush();
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

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
        std::cerr << "Invalid address\n";
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server. Type 'bye' to exit.\n";

    std::thread recvThread(receiveMessages, sock);
    recvThread.detach();

    // Send messages
    std::string message;
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, message);

        send(sock, message.c_str(), message.size(), 0);

        if (message == "bye") {
            break;
        }
    }

    close(sock);
    return 0;
}
