#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    /* Step 1: Create a socket  (TCP here)
    sockfd = socket(domain,type,protocol)*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /*Feature build here a setsokopt so it let e reuse it */

    // Step 2: Set up the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");  // Localhost

    // Step 3: Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    printf("✅ Successfully connected to the server!\n");

    // Step 4: Close the socket
    close(sockfd);
    return 0;
}
