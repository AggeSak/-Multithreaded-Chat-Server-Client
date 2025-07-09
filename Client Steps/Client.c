#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // 1. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // Server must match this port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    // 3. Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("✅ Connected to server at 127.0.0.1:8080\n");
    printf("Type a message ('exit' to quit):\n");

    // 4. Loop to send and receive messages
    while (1) {
        printf("You: ");
        fflush(stdout);

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        // Exit condition
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Send to server
        send(sockfd, input, strlen(input), 0);

        // Clear buffer and receive reply
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Server disconnected or error.\n");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate received data
        printf("📨 Server: %s\n", buffer);
    }

    // 5. Close socket
    close(sockfd);
    printf("Connection closed.\n");

    return 0;
}
