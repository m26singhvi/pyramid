#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "cli_commands.h"
#include "common.h"
#include "tlv.h"

int cli_fd;

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

unsigned int handle_data(Tlv_element tlv)
{
   switch(tlv.type)
   {
    case CLI_DATA:
    {
     printf("Message Received : \n %s", tlv.value);
     return 0;
    }
    case GOOD_BYE:
    {
      printf("Message Complete \n");
      return 1;
    } 
    default :
    {
     printf("%s : unknown Attribute, can't handle ", __func__);
     return -1;
    }
   }
}


void  receive_data ()
{
    printf("\nReceive data");
    unsigned int done = 0;

    while (1)
    {
        ssize_t count;
        char buf[ONE_KB];
        int flags = 0;

//        count = read (client_fd, buf, sizeof buf);
        count = recv(cli_fd, buf, sizeof buf, flags);
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
            //break;
        }
        else if (count == 0)
        {
            /* End of file. The remote has closed the
             * connection. */
            done = 1;
            break;
        }

	if (count > 0) {
            printf("Got some data on an existing fd %d\n",cli_fd);
            Tlv_element tlv = decode(buf, count);
            done = handle_data(tlv);
            if (done == 1)
               break;
	}
   
        /* Write the buffer to standard output */
/*        int s = write (1, tlv.value, tlv.length);
        if (s == -1)
        {
            perror ("write");
           abort ();
        }*/
    }

        cli_main(cli_fd);
}



/*
 * send_data ()
 *
 * This is a temp func for testing.
 */
void request_cli_data(int cli_type)
{
    char buffer[8];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d", cli_type);
    printf("\nFetching data from server database.: %s ", buffer);

        int len = strlen(buffer);
        int sent = 0;
        printf("\nLen : %d", len);
        while (len > 0) {
            Buffer buf;
            char payload[1024];
            memset(payload, 0, 1024);
            buf.payload = payload;
            buf.length = 0;
            int encoded_len = encode(CLI_DATA, (void *)buffer , len, &buf);
            if ((sent = send(cli_fd, payload, encoded_len, 0)) == -1) {
                report_error_and_terminate("Failed to send data");
            } else {
                len -= sent;
            }
        }
        printf("\nData Sent");
}



void cli_clear_screen(void) {
        system("clear");
}

void cli_help(void) {
  printf("\nList of Available Commands : \n");
  printf("\n\tshow multicast groups");
  printf("\n\tshow clients all");
  printf("\n\thelp");
  printf("\n\tclear");
}

void cli_print_multicast_groups(void) {
        printf("\nAvailable Multicast groups list:\n");
	request_cli_data(SHOW_MULTICAST_GROUPS);
	receive_data();
}

void cli_print_clients(void) {
	printf("\nList of available clients:\n");
	request_cli_data(SHOW_CLIENTS_ALL);
	receive_data();
}

/* TO-DO */
void cli_print_multicast_group_clients(int group_id) {
        printf("\nFollowing clients are available in multicast group %d\n", group_id);
        printf("\n*****Under Development*****\n");
}

void parse_cli(char *cli_string) {
        if (!strcmp(cli_string, "show multicast groups")){
                cli_print_multicast_groups();
        }else if(!strcmp(cli_string, "show clients all")){
                cli_print_clients();
        }else if(!strcmp(cli_string, "help")) {
                cli_help();
        }else if(!strcmp(cli_string, "clear")) {
                cli_clear_screen();
        }else {
                printf("\nCommand Not found!\n");
                printf("\nEnter \"help\" command to check the list of available commands.");
        }
}

//To-Do : when server starts, open a terminal and call this function
void cli_main(int fd){
        char str[50];
	cli_fd = fd;
	make_socket_non_blocking(fd);
        printf("\n ************ Pyramid CLI Interface *************\n\n");

        while(true) {
                printf("\nPYRAMID# ");
		gets(str);
                if(!strcmp(str,"exit")){
                        break;
                }else if(!strcmp(str, "")){
			continue;
		}
                parse_cli(str);
                continue;
        }
        printf("\n***** End *****\n");
}

