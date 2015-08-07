#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include "server.h"
#include "common.h" 

static int init_server_socket(int server_p)
{
    int server_port = htons(server_p);
    int server_fd;
    struct sockaddr_in server_s_addr= {0};

    IPV4_SOCKADDR(server_s_addr, AF_INET, server_port, htonl(INADDR_ANY));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create FD");
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *) &server_s_addr,
                sizeof(server_s_addr)) == -1) {
        perror("Failed to bind");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

/*
 * listen_for_client_connections ()
 *
 * This function accepts connections
 * and process information to join groups.
 */
static void listen_for_client_connections(int server_fd)
{
    pthread_t tid[BACKLOG];
    struct sockaddr_in client_s_addr={0};
    // TODO: Need to handle cases were client terminates/timesout
    if (listen(server_fd, BACKLOG) == -1) {
        perror("Failed to listen for connections");
        return;
    }

        
    for(int i = 0 ; i < BACKLOG; i++) {
        socklen_t client_addr_len = sizeof(client_s_addr);
        int client_fd = accept(server_fd, (struct sockaddr *) &client_s_addr,
                &client_addr_len);
        if (client_fd == -1) {
            perror("Failed to accept");
        }

        printf("Accepted connection from:\n");
        pthread_create(&tid[i], NULL, &receive_data, &client_fd);
    }
}

static void server_cleanup(int server_tid){
    close(server_tid);
}

static int validate_port_number(char *port){
    char *temp = '\0';
    long int server_port = strtol(port, &temp, 10);
    if(*temp != '\0'  
        || server_port == LONG_MIN 
        || server_port == LONG_MAX
        || (server_port >= 0 && server_port <= 1023) 
        ){
        /* See man strol for more info*/
        return 0;
    }
    return server_port;
}

int main (int argc, char* argv[]) {
    int server_port ;
    /*Validate and convert input port */
    if(argc != 2 
        || (server_port = validate_port_number(argv[1])) == 0){
        printf("Invalid port entered. Assuming port = 51729\n");
        fflush(stdout);
        server_port = 51729;
    }
    /* Ininitialize server socket */
    int server_tid = init_server_socket(server_port);

    if(server_tid != -1){
        fflush(stdout);
        printf("Server started at port : %d\n", server_port);
        /* Listen for new connections */
        listen_for_client_connections(server_tid);

        /* Waiting for all threads to complete */
        pthread_join(server_tid, NULL);
    }

    server_cleanup(server_tid);
    return 0;
}


/*
 * receive_data ()
 *
 * This is a temp func for testing.
 * Need to be removed once we have a thread that
 * pools on all client fds
 */
void * receive_data (void* args)
{
    int client_fd = *((int *) args);
    char buffer[ONE_KB];

    while (1) {
        int received = recv(client_fd, buffer, ONE_KB - 1, 0);
        if (received == -1) { 		// error
            perror("Unable to receive data");
            continue;
        } else if (received > 0) {	// data received
            buffer[received] = '\0';
            printf("%d: %s", client_fd, buffer);
        } else if (received == 0) {	// shutdown
            break;
        }
    }

    close(client_fd); 

    return NULL;
}
