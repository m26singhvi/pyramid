#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"
#include "../../common/common.h" // FIXME: take care of relative addressing

int
main (int argc, char* argv[]) {
    pthread_t server_tid;
    struct thread_args t_args;

    t_args.argc = argc;
    t_args.argv = argv;

    if (argc != 2) {
	printf("Usage: server <port>\n");
	exit(EXIT_FAILURE);
    }

    // Create server thread
    pthread_create(&server_tid, NULL, &server, &t_args);

    // TODO: Create client data handling thread

    // TODO: Create logging thread

    // TODO: Create CLI console thread

    // Waiting for server thread to complete
    pthread_join(server_tid, NULL);

    return 0;
}

/*
 * server ()
 *
 * This is the main server thread which accepts connections
 * and process information to join groups.
 */
void*
server (void* args)
{
    int server_fd, client_fd;
    int server_port = htons(atoi(((struct thread_args *)args)->argv[1]));
    //int no_of_args = ((struct thread_args *)args)->argc;
    struct sockaddr_in server_s_addr, client_s_addr;
    pthread_t tid[BACKLOG];
    int i;

    memset(&server_s_addr, 0, sizeof(server_s_addr));
    IPV4_SOCKADDR(server_s_addr, AF_INET, server_port, htonl(INADDR_ANY));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
	report_error_and_terminate("Failed to create FD");
    }

    if (bind(server_fd, (struct sockaddr *) &server_s_addr,
	sizeof(server_s_addr)) == -1) {
	report_error_and_terminate("Failed to bind");
    }

    if (listen(server_fd, BACKLOG) == -1) {
	report_error_and_terminate("Failed to listen for connections");
    }

    // TODO: Need to handle cases were client terminates/timesout
    for (i = 0; i < BACKLOG; i++) {
	socklen_t client_addr_len = sizeof(client_s_addr);

	client_fd = accept(server_fd, (struct sockaddr *) &client_s_addr,
			   &client_addr_len);
	if (client_fd == -1) {
	    perror("Failed to accept");
	}

	printf("Accepted connection from:\n");
	// FIXME: Remove this close once we store fds in array
	pthread_create(&tid[i], NULL, &receive_data, &client_fd);
    }

    return NULL;
}

/*
 * receive_data ()
 *
 * This is a temp func for testing.
 * Need to be removed once we have a thread that
 * pools on all client fds
 */
void*
receive_data (void* args)
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
