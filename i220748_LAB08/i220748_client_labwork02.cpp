#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3001

using namespace std;

void client() {
    int sock;
    struct sockaddr_in serv_addr;
    int num1, num2, result;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Socket creation failed." << endl;
        return;
    }

    // Set up server address struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address." << endl;
        close(sock);
        return;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        cerr << "Connection failed." << endl;
        close(sock);
        return;
    }

    // user input
    cout << "Enter a number: ";
    cin >> num1;
    send(sock, &num1, sizeof(num1), 0);

    cout << "Enter another number: ";
    cin >> num2;
    send(sock, &num2, sizeof(num2), 0);

    recv(sock, &result, sizeof(result), 0);
    cout << "Server result: " << result << endl;

    close(sock);
}

int main() {
    client();
    return 0;
}
