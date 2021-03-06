#include <sys/types.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "tlv.h"
#include "common.h"
#include "cli_commands.h"
#include "api.h"
#include "central_server.h"
#include "dynamic_lib_interface.h"

extern int inet_aton(const char *cp, struct in_addr *inp);

void receive_exec_request (int server_fd);
static void send_data(int fd, char *buffer, int ALGO);
static void send_joining_groups(int fd, uint32_t *groups, int numgroups);
bool handleServerMessage(char *buf, int recvLen, int server_fd);

enum client_error_msgs {
    CL_INVALID_IP,
    CL_INVALID_PORT,
    CL_LAST /* Should be always last */
};

/*
 * client_print_usage ()
 *
 * Print the command usage in case a wrong command was issued.
 */
static inline void
client_print_usage (void)
{
    printf("Usage: client <server ip> <server port> group-ids...\n");
}

/*
 * client_print_error_msgs ()
 *
 * Print the error messages generated by the client.
 */
static void
client_print_error_msgs (enum client_error_msgs error, char *msg)
{
    switch(error) {
    case CL_INVALID_IP:
	printf("Invalid IP address %s\n", msg);
	client_print_usage();
	break;
    case CL_INVALID_PORT:
	printf("Invalid port number %s\n", msg);
	client_print_usage();
	break;
    default:
	break;
    }
}

/*
 * client_input_sanity_check ()
 *
 * Verifies that the command line args passed are appropriate or not.
 * If sucessfully verified the server IP and port is stored in
 * sip and sp, respectively.
 */
static enum boolean 
client_input_sanity_check (int argc,
			   char *argv[],
			   in_addr_t *sip,
			   in_port_t *sp)
{
    struct in_addr addr;
    int port = 0;
    char *as = argv[1];
    char *ps = argv[2];
//    printf("\nAddress : %s", as);
//    printf("\nPort : %s", ps);

    int i = 0;

    if (argc <= 3) {
	client_print_usage();
	return FALSE;
    }

    // Check if the address is well formed IP Address or not
    if (inet_aton(as, &addr) == 0) {
	client_print_error_msgs(CL_INVALID_IP, as);
	return FALSE;
    }
    *sip = addr.s_addr;

    while (ps[i]) {
	if (!isdigit(ps[i])) {
	    client_print_error_msgs(CL_INVALID_PORT, ps);
	    return FALSE;
	}
	i++;
    }
    port = atoi(ps);
    if (port > USHRT_MAX) {
	client_print_error_msgs(CL_INVALID_PORT, ps);
	return FALSE;
    }
    *sp = htons(port);

    return TRUE;
}

static inline int
client_create_tcp_socket (void)
{
    int fd = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
	report_error_and_terminate("Failed to create socket");
    }
    return fd;
}

static inline void
client_close_fd (int fd)
{
    if (close(fd) == -1) {
	perror(__FUNCTION__);
    }
}

void
client_connect (int cfd, in_addr_t sip, in_port_t sp)
{

    struct sockaddr_in server_s_addr;

    memset(&server_s_addr, 0, sizeof(server_s_addr));
    IPV4_SOCKADDR(server_s_addr, AF_INET, sp, sip);

    if (connect(cfd, (struct sockaddr *) &server_s_addr,
	sizeof(server_s_addr)) == -1) {
	perror("Failed to connect");
	client_close_fd(cfd);
	exit(EXIT_FAILURE);
    }

}

int
main (int argc, char* argv[])
{
    int client_s_fd;
    in_addr_t sip = 0;
    in_port_t sp = 0;

    // TODO: Create keep alive thread. See if this can be merged with the client thread

    // Sanity check the command line inputs.
    if (client_input_sanity_check(argc, argv, &sip, &sp) == FALSE) {
	exit(EXIT_FAILURE);
    }
 
    client_s_fd = client_create_tcp_socket();
    client_connect(client_s_fd, sip, sp);
    
    if(argc == 4 && !strcmp(argv[3], "CLI"))
    {
	cli_main(client_s_fd);
    } else {

        int numgroups = 0;
        uint32_t groups[255];
        for (int i = 0 ; i < (argc-3); i++) 
        {
           groups[i] = atoi(argv[i+3]);
           numgroups = numgroups + 1;
        }
        send_joining_groups (client_s_fd, groups, numgroups);
        printf("\nWaiting for task from server"); 
        // Take user input and send it to server
        //send_data(client_s_fd);
	//Receive task from user and send back result
	receive_exec_request(client_s_fd);
//        printf("\nDone Request");
    }	
    client_close_fd(client_s_fd);

    return 0;
}

static enum boolean
client_get_file_from_ctrl_repo (const char *cntrl_repo_path,
				const char *local_path)
{
    char buffer[MAX_SSH_CMD_SIZE] = {0};
    snprintf(buffer, MAX_SSH_CMD_SIZE, "wget http://%s -O %s",
			cntrl_repo_path, local_path);

    // add debug here
#if 0
    if (system(buffer) == -1) {
	return FALSE;
    }
#endif
    if (system(buffer)) {};
    sleep(1);

    return TRUE;
}

enum boolean
client_open_file_to_read_max (const char *filename, char result[])
{
    FILE *fp = fopen(filename, "r");
    enum boolean ret = TRUE;

    if (fp == NULL) {
	return FALSE;
    }

    if (fgets(result, 100, fp) == NULL) {
	ret = FALSE;
    }

    fclose(fp);

    return ret;
}

static inline char *
get_filename_from_path (char path[])
{
    char *filename = NULL;
    char *save = NULL;
    char *current = NULL;

    filename = strtok_r(path, "/", &save);
    while ((current = strtok_r(NULL, "/", &save)) != NULL) {
	filename = current;
    }

    return filename;
}

static inline enum boolean
client_generate_in_out_filenames_from_path (
			char in[], char out[], int size, char path[])
{
    char *central_server_filename = get_filename_from_path(path);
    enum boolean ret = FALSE;
    char * in_f = ""; //"input_";
    char * out_f = "output_";

    if (central_server_filename) {
	int path_len = strnlen(central_server_filename, size);
	int in_len = strnlen(in_f, size);
	int out_len = strnlen(out_f, size);
	if (((snprintf(in, size, "%s%s", in_f, central_server_filename)) == path_len + in_len) &&
	    (snprintf(out, size, "%s%s", out_f, central_server_filename) == path_len + out_len)) {
	    ret = TRUE;
	}
    }

    //add debug here

    return ret;
}

static void 
handle_exec_data(int server_fd, Tlv tlv)
{

   char buffer[100] = {0};
   char in_file[100] = {0};
   char out_file[100] = {0};
   strncpy(buffer, tlv.value, 100);
// Add debug here
   switch(tlv.type)
   {
        case ALGO_MAX:
	{
	    //CALL MAX API
	    // Add debug here
            api_status_t (*main_api)(void *input, void *output, api_id_t id);
	    void * plugin_handle =NULL;
            ASSIGN_FUNC_PTR("main_api",main_api,&plugin_handle);
            if((client_generate_in_out_filenames_from_path(in_file, out_file, 100, buffer) == TRUE) &&
	       (client_get_file_from_ctrl_repo(tlv.value, in_file) == TRUE) &&
	       (main_api(in_file, out_file, FIND_MAX) == API_SUCCESS) &&
	       (client_open_file_to_read_max(out_file, buffer) == TRUE)) {
                send_data(server_fd, buffer, ALGO_MAX);
                printf("Maximum number sent to server\n");
            } else {
                send_data(server_fd, "\nClientErrorType\n", ALGO_ERROR);
            }
	    dlclose(plugin_handle);
	    break;
	}
        case ALGO_SORT:
	{
	    printf("\nRequest Received to sort a File\n");
	    break;
	}
        default :
     	   printf("%s : unknown Attribute, can't handle ", __func__);
        break;
   }
}


/*
 * receive_data ()
 *
 */
void  
receive_exec_request (int server_fd)
{
 //   printf("\nReceive data from fd %d\n", server_fd);
    int done = 0;

    while (1)
    {
        ssize_t count;
        char buf[ONE_KB];
        int flags = 0;

        count = recv(server_fd, buf, sizeof buf, flags);
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
        //       printf("Got some data on an existing fd %d\n",server_fd);

        handleServerMessage(buf, count, server_fd);
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
        printf("No data on descriptor %d\n", server_fd);

    }
}

bool handleServerMessage(char *buf, int recvLen, int server_fd)
{
   int lenFromHdr = ntohl(*(uint32_t *) buf);
   int curSegLen = lenFromHdr;
   bool done = false;
   if (lenFromHdr > recvLen)
   {
     printf("Fragmented Packet");
     return true;
   }
   else 
   { 
     while(recvLen > 0)
     {
      lenFromHdr = ntohl(*(uint32_t *) buf);
      curSegLen = lenFromHdr; 
      buf = buf + 4;
      lenFromHdr = lenFromHdr - 4;
      while (lenFromHdr > 0)
      {
       Tlv tlv = decode(buf, lenFromHdr, NULL);
       lenFromHdr = lenFromHdr - tlv.length - 4;
       buf = buf + tlv.length + 4;
       handle_exec_data(server_fd, tlv);
      }
      recvLen = recvLen - curSegLen;
      
     }
    } 

  return done;
}

/*
 * send_data ()
 *
 * This is a temp func for testing.
 */
static void
send_data (int fd, char *buffer, int ALGO)
{
	int len = strlen(buffer);
	int sent = 0;
	while (len > 0) {
            Buffer buf;
            char payload[1024];
            memset(payload, 0, 1024);
            buf.payload = payload;
            buf.length = 0;
            int encoded_len = encode(ALGO, (void *)buffer , len, &buf);
	    if ((sent = send(fd, payload, encoded_len, 0)) == -1) {
		report_error_and_terminate("Failed to send data");
	    } else {
		len -= sent;
	    }
	}
}

/*
 * send_joining_groups ()
 *
 * This is a temp func for testing.
 */
static void
send_joining_groups (int fd, uint32_t *groups, int numgroups)
{
    int sent = 0;
    Buffer buf;
    char payload[1024];
    memset(payload, 0, 1024);
    buf.payload = payload;
    buf.length = 0;
    int encoded_len = encode(JOIN_GROUP, (void *)groups, numgroups, &buf);
	   
     if ((sent = send(fd, buf.payload, encoded_len, 0)) == -1) 
        report_error_and_terminate("Failed to send data");
     else
       printf("\nRequest sent to server for joining groups.");
}
