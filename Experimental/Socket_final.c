#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {

    /* (1) Socket Creation
       sockfd = socket(domain, type, protocol); */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("❌ Not Socket Creation");
        return 1;
    }

    /* (2) setsockopt()  
       Let me reuse the port even if it’s in TIME_WAIT state */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("❌ setsockopt failed");
        return 1;
    }

    /* (3) Define server address struct */
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    server_addr.sin_port = htons(4242);        // Port

    /* (4) Bind */
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("❌ Bind failed");
        return 1;
    }

    /* (5) Listen */
    if (listen(sockfd, 5) < 0) {
        perror("❌ Listen failed");
        return 1;
    }

    printf("✅ Server listening on port 4242...\n");

    /* (6) Accept */
    int new_socket = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
    if (new_socket < 0) {
        perror("❌ Accept failed");
        return 1;
    }

    printf("✅ Connection accepted from client!\n");

    /* (7) Send / Receive */
    char buffer[1024] = {0};
    recv(new_socket, buffer, sizeof(buffer), 0);  // Receive
    printf("📩 Received: %s\n", buffer);

    send(new_socket, "Hello from server!", strlen("Hello from server!"), 0); // Send

    /* (8) Close */
    close(new_socket);
    close(sockfd);

    return 0;
}
