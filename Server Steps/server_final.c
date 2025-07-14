#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 1024

int server_fd;
int running = 1;

void *console_listener(void *arg) {
    char input[BUFFER_SIZE];
    while (running) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (strncmp(input, "close", 5) == 0) {
                printf("Shutdown command received.\n");
                running = 0;
                // Close server_fd to unblock accept()
                close(server_fd);
                break;
            }
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening on port 8080...\n");

    // Start console listener thread
    pthread_t console_thread;
    pthread_create(&console_thread, NULL, console_listener, NULL);

    while (running) {
        // 4. Accept (will unblock when server_fd is closed)
        int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_fd < 0) {
            if (!running) break;  // accept failed because we closed the socket to exit
            perror("Accept");
            continue;
        }
        printf("Client connected!\n");

        // 5. Read
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received from client: %s\n", buffer);

            if (strncmp(buffer, "hello", 5) == 0) {
                send(client_fd, "hello\n", 6, 0);
                printf("Sent: hello\n");

            } else if (strncmp(buffer, "hi", 2) == 0) {
                send(client_fd, "Hi there!\n", 10, 0);
                printf("Sent: Hi there!\n");

            } else if (strncmp(buffer, "time", 4) == 0) {
                time_t now = time(NULL);
                char time_str[128];
                strftime(time_str, sizeof(time_str), "Current time: %Y-%m-%d %H:%M:%S\n", localtime(&now));
                send(client_fd, time_str, strlen(time_str), 0);
                printf("Sent: %s", time_str);

            } else {
                send(client_fd, "unknown command\n", 17, 0);
                printf("Sent: unknown command\n");
            }
        }

        close(client_fd);
    }

    printf("Server shutting down...\n");
    pthread_join(console_thread, NULL);

    return 0;
}
