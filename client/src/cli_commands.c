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
bool init_cli_interface = false;

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
   switch(tlv.type) {
   case CLI_DATA:
   {
       printf("\n%s", tlv.value);
       return 0;
   }
   case GOOD_BYE:
   {
       //printf("Message Complete \n");
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
    //printf("\nReceive data");
    unsigned int done = 0;

    while (1)
    {
        ssize_t count;
        char buf[ONE_KB];
        int flags = 0;

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
            //printf("Got some data on an existing fd %d\n",cli_fd);
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

    int len = strlen(buffer);
    int sent = 0;
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
}



void cli_clear_screen(void) {
   if(system("clear") == -1) {
	report_error_and_terminate("\nFailed to clear screen");
   }	
}

void cli_help(void) {
    printf("\nList of Available Commands : \n");
	 printf("\n Debug Commands:");
		printf("\n\tshow multicast groups");
		printf("\n\tshow clients all");
		printf("\n\thelp");
		printf("\n\tclear");
		printf("\n\tshow job-details <job_id>");
		printf("\n\tshow job-result <job-id>");
		
	 printf("\n Config Commands:");	
		printf("\n\tset repository-address <ip-address/path>");
		printf("\n\tset job-result-queue size <size>");
		
	 printf("\n Execute Commands:");
		printf("\n\texecute <TASK> file <file_name> <multicast_group_id>");
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

void cli_print_job_details(int job_id) {
    printf("\nJob details for job: %d\n", job_id);
    printf("\n\tInput: ");
    printf("\n\tTask: ");
    printf("\n\tStatus: ");
    printf("\n");
}

void cli_print_job_result(int job_id) {
    printf("\nResult of Job: %d\n", job_id);
    printf("\n\tStatus: ");
    printf("\n\tResult: ");
    printf("\n");
}

void cli_set_repository_address(char *address) {
    printf("\nCentral data repository address updated: %s", address);
    printf("\n");
}

void cli_set_job_result_queue_size(int size) {
    printf("\nJob result queue size updated to: %d ", size);
    printf("\n");
}

void cli_exec_task(int taskid, char *file, int groupid) {
    printf("\nJob submitted for execution!");
    printf("Details : %d :  %s : %d", taskid, file, groupid);
    printf("\n");
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
	}else if(!strncmp(cli_string, "show job-details", strlen("show job-details"))) {
	        cli_print_job_details(1);
	}else if(!strncmp(cli_string, "show job-result", strlen("show job-result"))) {
		cli_print_job_result(1);
	}else if(!strncmp(cli_string, "exec", strlen("exec"))) {	
    		cli_exec_task(1, "abc", 1);	
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
    if(!init_cli_interface) {
    	printf("\n ************ Pyramid CLI Interface *************\n\n");
        init_cli_interface = true;
    }

    while(true) {
        printf("\nPYRAMID# ");
	if(fgets(str, sizeof(str), stdin) != NULL){
	   int len = strlen(str);
	   str[len-1] = '\0';
           if(!strcmp(str,"exit")){
                break;
           }else if(!strcmp(str, "")){
		continue;
 	   }
           parse_cli(str);
           continue;
	}
    }
    printf("\n***** End *****\n");
}

