#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define END_MARKER "<EOF>"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(port);       

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) { 
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("[+] Server started successfully.\n");
    printf("[+] Listening on port %d...\n", port);

    while (1) {
        printf("\n[+] Waiting for a new client to connect...\n");
        
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Accept failed");
            continue; // nashod accept konim vaymisim ta hal beshe servera nemibandim 
        }

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        printf("[+] A new client connected from: %s\n", client_ip);

        char command[BUFFER_SIZE];
        char response[BUFFER_SIZE];

        while (1) {
            printf("%s $ ", client_ip);
            memset(command, 0, BUFFER_SIZE); // flush command
            
            if (fgets(command, BUFFER_SIZE, stdin) == NULL) continue;
            
            command[strcspn(command, "\n")] = 0;

            if (strlen(command) == 0) continue;

            // alan in client kharej mishe va montazer baghiye mimone 
            if (strcmp(command, "exit") == 0) {
                printf("Closing connection with current client...\n");
                break; 
            }

            int bytes_sent = send(client_fd, command, strlen(command), 0);
            if (bytes_sent == -1) {
                perror("Send failed");
                break; 
            }

            int bytes_received;
            int connection_closed = 0; 

            while (1) {
                memset(response, 0, BUFFER_SIZE);
                bytes_received = recv(client_fd, response, BUFFER_SIZE - 1, 0);
                
                if (bytes_received <= 0) {
                    printf("\n[-] Connection closed by client.\n");
                    connection_closed = 1; 
                    break; 
                }

                char *end_ptr = strstr(response, END_MARKER);
                if (end_ptr != NULL) {
                    *end_ptr = '\0'; 
                    printf("%s", response);
                    break; 
                } else {
                    printf("%s", response); 
                }
            }
            
            // age ghat shode bodim connectiona mibandim
            if (connection_closed) {
                break; 
            }
        }
        
        // client ham ghat mikonim bad payan 
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
