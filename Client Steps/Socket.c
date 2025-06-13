#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully! FD: %d\n", sockfd);

    close(sockfd);  // Don't forget to close sockets when done
    return 0;
}
