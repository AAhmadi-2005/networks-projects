#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define MAX_CLIENTS 10 // max tedad client 
#define BUFFER_SIZE 4096

// sakhtar data e ke az client negahmidarim 
typedef struct {
    int id;
    int socket;
    char ip[INET_ADDRSTRLEN];
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
// baray handle sendall ersal payam har client joda (thread hay clienta moghe sendall ghati nashan)
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; 
int current_client_id = -1; // defalt clienti entekhab nashode 

// ersal be ye client khas 
void send_to_client(int sock, const char* cmd) {
    send(sock, cmd, strlen(cmd), 0);
}

// thread daryaft payam
void *client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    
    // ye buffer bara gereftan kole javab ghable az neveshtanesh to cmd server
    // (ke bara sendall thread ha ghati nashe )
    char *full_response = malloc(1);
    full_response[0] = '\0';
    int total_len = 0;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read <= 0) {
            pthread_mutex_lock(&clients_mutex);
            // ghfle safhe baray inke ye payam kamel ersal beshe 
            pthread_mutex_lock(&print_mutex);
            printf("\n[-] Client %d (%s) disconnected.\nServer> ", client->id, client->ip);
            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
            
            client->active = 0;
            if (current_client_id == client->id) current_client_id = -1;
            close(client->socket);
            pthread_mutex_unlock(&clients_mutex);
            break;
        }

        // in tike kolan baray handle naneveshtan eof tah payama bara servere 
        int end_of_msg = 0;
        char *eof_ptr = strstr(buffer, "<EOF>");
        if (eof_ptr != NULL) {
            *eof_ptr = '\0'; // in bara fgete ke tamom she 
            end_of_msg = 1;
        }

        // har teke payama ta be tah payam beresim ezafe mikonim to buffer 
        total_len += strlen(buffer);
        full_response = realloc(full_response, total_len + 1);
        strcat(full_response, buffer);

        // be tahesh ke residim mirim chap mikonim 
        if (end_of_msg) {
            // safe ra negah midarim ta thread hay dige nayad vasatesh 
            pthread_mutex_lock(&print_mutex);
            
            printf("\n[Response from Client %d]:\n%s\nServer> ", client->id, full_response);
            fflush(stdout);
            
            pthread_mutex_unlock(&print_mutex); // safe ra baz mikonim 

            // hafeze ra va mikonim 
            free(full_response);
            full_response = malloc(1);
            full_response[0] = '\0';
            total_len = 0;
        }
    }
    
    free(full_response);
    return NULL;
}

// baray modiriat server
void *admin_console(void *arg) {
    char input[BUFFER_SIZE];
    
    sleep(1); // ye sleep baray inke amade beshe server 
    
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. list    : List all connected clients\n");
        printf("2. select  : Select a client (e.g., select 1)\n");
        printf("3. sendall : Send command to all (e.g., sendall ls)\n");
        printf("4. [cmd]   : Send direct command to selected client\n");
        printf("Server> ");
        
        memset(input, 0, BUFFER_SIZE);
        if (!fgets(input, BUFFER_SIZE, stdin)) continue;
        input[strcspn(input, "\n")] = 0; // حذف newline

        if (strlen(input) == 0) continue;

        if (strncmp(input, "list", 4) == 0) {
            pthread_mutex_lock(&clients_mutex); // lock mishe ta kase digei fellan avazesh nakone 
            printf("\nConnected Clients:\n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    printf("ID: %d | IP: %s\n", clients[i].id, clients[i].ip);
                }
            }
            pthread_mutex_unlock(&clients_mutex);// karemon tamom shode unlock mishe 
        } 
        else if (strncmp(input, "select ", 7) == 0) {
            int target_id = atoi(input + 7);
            int found = 0;
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active && clients[i].id == target_id) {
                    current_client_id = target_id;
                    found = 1;
                    printf("[+] Selected client %d\n", target_id);
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            if (!found) printf("[-] Client not found.\n");
        }
        else if (strncmp(input, "sendall ", 8) == 0) {
            char *cmd_to_send = input + 8;
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    send_to_client(clients[i].socket, cmd_to_send);
                }
            }
            printf("[+] Command sent to all clients.\n");
            pthread_mutex_unlock(&clients_mutex);
        }
        else {
            // dastor mamoli baray on clienti ke entkhab shode 
            if (current_client_id == -1) { // kasi entekhab nashode 
                printf("[-] No client selected. Use 'select <id>' or 'sendall <cmd>'.\n");
            } else {
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].active && clients[i].id == current_client_id) {
                        send_to_client(clients[i].socket, input);
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // age yeki ghat shod server crash nakone 
    // az onjayi ke alan darim ro ye system ejra mikonim a takhir nadarim toy loop rec gir nemikonim 
    // (to fget gir mimonim ) {mage inke client toy zamani ke dare payam mifrese ghat she ke inja chon
    // takhir kheyli kame amalan momken ni }
    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // arrray clienta flush A amade mikonim
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].active = 0;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 10);

    printf("[+] Server listening on port %d\n", port);

    // thered admin server baray handel payama ra rah mindazim
    pthread_t admin_tid;
    pthread_create(&admin_tid, NULL, admin_console, NULL);

    int id_counter = 1;

    // while accept client 
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) continue;

        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                clients[i].socket = client_sock;
                clients[i].id = id_counter++;
                inet_ntop(AF_INET, &client_addr.sin_addr, clients[i].ip, INET_ADDRSTRLEN);
                clients[i].active = 1;
                printf("\n[+] New Client Connected: ID %d | IP: %s\nServer> ", clients[i].id, clients[i].ip);
                fflush(stdout);
                
                pthread_t client_tid;
                pthread_create(&client_tid, NULL, client_handler, &clients[i]); // client ezafe shode pa thread behsh midim
                pthread_detach(client_tid);
                break;
            }
        }
        if (i == MAX_CLIENTS) {
            printf("\n[-] Maximum clients reached. Rejecting.\nServer> ");
            close(client_sock);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_sock);
    return 0;
}
