#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    // (1) Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // (2) Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242); // same port as server
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // local

    // (3) Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // (4) Send message
    send(sockfd, "Hello from client!", strlen("Hello from client!"), 0);

    // (5) Receive message
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("📨 From server: %s\n", buffer);

    // (6) Close
    close(sockfd);
    return 0;
}
