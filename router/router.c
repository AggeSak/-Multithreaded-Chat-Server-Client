#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define LISTEN_PORT 8080          // Proxy listens here
#define TARGET_PORT 9000          // Destination server (Server B)
#define BUFFER_SIZE 4096

int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    if (listen(sockfd, 1) < 0) {
        perror("listen"); exit(1);
    }

    return sockfd;
}

int connect_to_target(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in target_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &target_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&target_addr, sizeof(target_addr)) < 0) {
        perror("connect to target"); exit(1);
    }

    return sockfd;
}

void proxy_data(int client_fd, int target_fd) {
    char buffer[BUFFER_SIZE];
    fd_set fds;
    int maxfd = (client_fd > target_fd ? client_fd : target_fd) + 1;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(target_fd, &fds);

        if (select(maxfd, &fds, NULL, NULL, NULL) < 0) {
            perror("select"); break;
        }

        if (FD_ISSET(client_fd, &fds)) {
            int n = read(client_fd, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            write(target_fd, buffer, n);
        }

        if (FD_ISSET(target_fd, &fds)) {
            int n = read(target_fd, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            write(client_fd, buffer, n);
        }
    }
}

int main() {
    int server_fd = create_server_socket(LISTEN_PORT);
    printf("Proxy listening on port %d\n", LISTEN_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) { perror("accept"); exit(1); }

    printf("Client connected\n");

    int target_fd = connect_to_target("127.0.0.1", TARGET_PORT);
    printf("Connected to target server on port %d\n", TARGET_PORT);

    proxy_data(client_fd, target_fd);

    close(client_fd);
    close(target_fd);
    close(server_fd);
    printf("Connection closed.\n");

    return 0;
}
