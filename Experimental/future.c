/*
 *just some code for something in the future, ignore
 * Multithreaded TCP Chat Server in C
 * - Check the tcp implement for the 2on proxu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 9000
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define NAME_LEN 32

typedef struct {
    struct sockaddr_in addr;
    int sockfd;
    int id;
    char name[NAME_LEN];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int uid = 10;

// Utility function to trim newline
void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) { // trim \n or \r
        if (arr[i] == '\n' || arr[i] == '\r') {
            arr[i] = '\0';
            break;
        }
    }
}

// Add client to clients array
void client_add(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove client from clients array
void client_remove(int id) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(clients[i]) {
            if(clients[i]->id == id) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to all clients except sender
void send_message(char *s, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(clients[i]) {
            if(clients[i]->id != sender_id) {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0) {
                    perror("ERROR: write to descriptor failed");
                    continue;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to a specific client by id
void send_private_message(char *s, int target_id) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(clients[i] && clients[i]->id == target_id) {
            if(write(clients[i]->sockfd, s, strlen(s)) < 0) {
                perror("ERROR: write to descriptor failed");
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// List all connected clients to a requester
void list_clients(int requester_id) {
    char s[BUFFER_SIZE];
    char list[BUFFER_SIZE] = "Connected users:\n";

    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(clients[i]) {
            snprintf(s, sizeof(s), "%s\n", clients[i]->name);
            strncat(list, s, sizeof(list) - strlen(list) - 1);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    send_private_message(list, requester_id);
}

// Handle communication with client
void *handle_client(void *arg) {
    char buff_out[BUFFER_SIZE];
    char name[NAME_LEN];
    int leave_flag = 0;

    client_t *cli = (client_t *)arg;

    // Name
    if(recv(cli->sockfd, name, NAME_LEN, 0) <= 0 || strlen(name) < 2 || strlen(name) >= NAME_LEN-1) {
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    } else {
        strcpy(cli->name, name);
        snprintf(buff_out, sizeof(buff_out), "%s has joined\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->id);
    }

    bzero(buff_out, BUFFER_SIZE);

    while(1) {
        if (leave_flag) {
            break;
        }

        int receive = recv(cli->sockfd, buff_out, BUFFER_SIZE, 0);
        if (receive > 0) {
            buff_out[receive] = '\0';
            str_trim_lf(buff_out, strlen(buff_out));

            // Handle commands
            if (buff_out[0] == '/') {
                // Command handling
                if (strncmp(buff_out, "/quit", 5) == 0) {
                    snprintf(buff_out, sizeof(buff_out), "%s has left\n", cli->name);
                    printf("%s", buff_out);
                    send_message(buff_out, cli->id);
                    leave_flag = 1;
                } else if (strncmp(buff_out, "/list", 5) == 0) {
                    list_clients(cli->id);
                } else if (strncmp(buff_out, "/msg ", 5) == 0) {
                    // Private message format: /msg <name> <message>
                    char *to_name = strtok(buff_out + 5, " ");
                    char *message = strtok(NULL, "\0");
                    if (to_name && message) {
                        pthread_mutex_lock(&clients_mutex);
                        int found = 0;
                        for (int i=0; i < MAX_CLIENTS; ++i) {
                            if (clients[i] && strcmp(clients[i]->name, to_name) == 0) {
                                char pm[BUFFER_SIZE];
                                snprintf(pm, sizeof(pm), "[PM from %s]: %s\n", cli->name, message);
                                send_private_message(pm, clients[i]->id);
                                found = 1;
                                break;
                            }
                        }
                        pthread_mutex_unlock(&clients_mutex);
                        if (!found) {
                            char err_msg[BUFFER_SIZE];
                            snprintf(err_msg, sizeof(err_msg), "User %s not found.\n", to_name);
                            send_private_message(err_msg, cli->id);
                        }
                    } else {
                        send_private_message("Invalid private message format. Use /msg <name> <message>\n", cli->id);
                    }
                } else {
                    send_private_message("Unknown command.\n", cli->id);
                }
            } else {
                // Broadcast message
                char message[BUFFER_SIZE];
                snprintf(message, sizeof(message), "%s: %s\n", cli->name, buff_out);
                printf("%s", message);
                send_message(message, cli->id);
            }

        } else if (receive == 0 || strcmp(buff_out, "/quit") == 0) {
            snprintf(buff_out, sizeof(buff_out), "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->id);
            leave_flag = 1;
        } else {
            perror("recv failed");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SIZE);
    }

    close(cli->sockfd);
    client_remove(cli->id);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}

int main() {
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid;

    // Socket settings
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // For reuse of address and port
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if(listen(listenfd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("=== Welcome to the Chat Server ===\n");

    while(1) {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        if(connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Check max clients
        pthread_mutex_lock(&clients_mutex);
        int client_count = 0;
        for (int i=0; i < MAX_CLIENTS; ++i) {
            if (clients[i]) client_count++;
        }
        pthread_mutex_unlock(&clients_mutex);

        if(client_count >= MAX_CLIENTS) {
            printf("Max clients reached. Rejecting: ");
            printf("%s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            close(connfd);
            continue;
        }

        // Client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->sockfd = connfd;
        cli->id = uid++;

        // Add client and create thread
        client_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // Reduce CPU usage
        sleep(1);
    }

    return 0;
}


