#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <sys/epoll.h>
#include <errno.h>
#include "server.h"

#include "common.h" 
#include "groups.h"
#include "tlv.h"
#include "server_helper.h"
#include "logging.h"
#include "jobs.h"
#include "dbg_server.h"

extern unsigned int g_groups[255];

#define MAX_EVENTS 2500 * 4

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

static int init_server_socket(int server_p)
{
    int server_port = htons(server_p);
    int server_fd;
    struct sockaddr_in server_s_addr; //= {0};

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
static void handle_new_client_connections(struct epoll_event ev, int epollfd, int server_fd)
{
    printf("\nHandle New Client Connection");
    while(1){
        struct sockaddr in_addr;
        socklen_t in_len;
        in_len = sizeof in_addr;
        int conn_sock = accept(server_fd,
                (struct sockaddr *) &in_addr, &in_len);
        if (conn_sock == -1) {
            if ((errno == EAGAIN) ||
                (errno == EWOULDBLOCK))
            {
                /* We have processed all incoming
                 * connections. */
                break;
            }
            else
            {
                perror ("accept");
                break;
            }
        }
        printf("New client started %d\n",conn_sock);
        if(make_socket_non_blocking(conn_sock) == -1) {
            break;
        }
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = conn_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
            perror("epoll_ctl: conn_sock");
            break;
        }
    }
}

/*
 * init_and_listen_epoll_events ()
 *
 * This function accepts connections
 * and process information to join groups.
 */
static void init_and_listen_epoll_events(int server_fd)
{
    struct epoll_event ev;
    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("Failed to listen for connections");
        return;
    }

    int epollfd = epoll_create1(0);
    struct epoll_event *events;

    if (epollfd == -1) {
        perror("epoll_create1");
        return;
    }
    ev.events = EPOLLIN ;/* | EPOLLET; For Edge trigger*/
    ev.data.fd = server_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        return;
    }
    events = calloc(MAX_EVENTS, sizeof ev);

    while(1){
        /*Wait for new epoll event*/
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        printf("Got some events nfds %d\n",nfds);
        if (nfds == -1) {
            perror("epoll_wait");
            return;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                handle_new_client_connections(ev, epollfd, server_fd);
            } else {
                receive_data(events[n].data.fd);
            }
        }
    }
    free(events);
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

static enum boolean
server_alloc_memory (void)
{
    uint mmg = server_get_max_multicast_groups() * sizeof(client_group_head);
    uint mhs = server_get_max_hashmap_size() * sizeof(client_info_head);

    dbg_server_memory("max_multicast_groups = %u, max_hashmap_size = %u\n",
		      mmg, mhs);

    multicast_groups = (client_group_head *) calloc(1, mmg);
    if (multicast_groups == NULL) {
	return FALSE;
    }

    client_hash_map = (client_info_head *) calloc(1, mhs);
    if (multicast_groups == NULL) {
	return FALSE;
    }

    return TRUE;
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

    if (server_alloc_memory() == FALSE) {
	print_error_and_terminate("Unable to allocate memory");
    }

    /* Ininitialize server socket */
    int server_tid = init_server_socket(server_port);

    if(server_tid != -1 && make_socket_non_blocking(server_tid) != -1){
        printf("Server started at port : %d\n", server_port);
        /* Listen for new connections */
        initializeJobDll();
        init_and_listen_epoll_events(server_tid);

        /* Waiting for all threads to complete */
    }

    server_cleanup(server_tid);
    return 0;
}


/*
 * receive_data ()
 *
 */
void  receive_data (int client_fd)
{
    client_info_head *cih = server_get_client_info_head(client_fd);
    printf("\nReceive data");
    int done = 0;

    while (1)
    {
        ssize_t count;
        char buf[ONE_KB];
	int flags = 0;

 	count = recv(client_fd, buf, sizeof buf, flags);
	buf[count] = '\0';
        if (count == -1)
        {
            /* If errno == EAGAIN, that means we have read all
             * data. So go back to the main loop. */
            if (errno != EAGAIN)
            {
                perror ("read");
                done = 1;
            }
            break;
        }
        else if (count == 0)
        {
            /* End of file. The remote has closed the
             * connection. */
            done = 1;
            break;
        }

        printf("\nGot some data on an existing fd %d\n",client_fd);
        Tlv tlv = decode(buf, count);
        handle_data(client_fd, tlv);
        /* Write the buffer to standard output */
        /*int s = write (1, tlv.value, tlv.length);
	printf("\nA.");
        if (s == -1)
        {
            perror ("write");
            abort ();
        }*/
    }

    if (done)
    {
        printf("Closed connection on descriptor %d\n", client_fd);
	server_del_one_client_fd(cih, client_fd);

        /* Closing the descriptor will make epoll remove it
         * from the set of descriptors which are monitored. */
        close (client_fd);
    }
}


void handle_data(int client_fd, Tlv tlv)
{
   client_info_head *cih = server_get_client_info_head(client_fd);
   switch(tlv.type)
   {
    case JOIN_GROUP:
    {
	struct sockaddr_in client_s_addr;
	socklen_t size = sizeof(client_s_addr);
	int res = getpeername(client_fd, (struct sockaddr *) &client_s_addr, &size);
	if (res == -1) {
	    perror("Failed to fetch peername");
	    return;
	}
	server_add_one_client_fd(cih, client_fd, &client_s_addr,
                        g_groups, tlv.length);
    }
    printf(" All groups joined \n"); 
    break;    

    case STRING_DATA:
    {
     printf("Message Received : \n %s", tlv.value); 
     break;
    }
    
    case CLI_DATA:
	sh_parse_cmd(client_fd, tlv.value, tlv.length);
	break;
    case ALGO_MAX:
         // TO DO : handle sub-result here
         // store the base path , free the client
         // and check if all the clients are done or not
         // if done, process the sub-results and remove the job
        printf("AlgoMaxResult : [%s] sending to FD : [%d]",tlv.value, client_fd );
	sh_send_job_result_to_cli(client_fd, tlv.value);
        printf("\nMessage Received [MAX]: \n %s", tlv.value); 	
        break;
    case ALGO_SORT:
        printf("\nReceived Algo SORT data from client:");
	break;
    case ALGO_ERROR:
         printf("This client %d couldn't do the job\n", client_fd);
         break;
    default : 
     printf("%s : unknown Attribute, can't handle ", __func__); 
    break;
   }
}
