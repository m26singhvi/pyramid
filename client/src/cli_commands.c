#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "cli_commands.h"
#include "common.h"
#include "tlv.h"
#include "logging.h"

#define CLI_MAX_WORDS 5

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

unsigned int handle_data(Tlv tlv)
{
//   printf("\nHandle_data");
   switch(tlv.type) {
   case CLI_DATA:
   {
       printf("\n %s", tlv.value);
       return 0;
   }
   case ALGO_MAX:
   {
	printf("\n %s", tlv.value);
	return 0;
   }
   case ALGO_SORT:
   {
	printf("\n %s", tlv.value);
	return 0;
   }
   case GOOD_BYE:
   {
       printf("\n---------- \n");
       return 1;
   } 
   default :
   {
       printf("\n%s : unknown Attribute, can't handle ", __func__);
       return -1;
   }
   }
}


void  receive_data ()
{
 //   printf("\nReceive CLI data");
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
 //           printf("Got some data on an existing fd %d\n",cli_fd);
            Tlv tlv = decode(buf, count);
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
void request_cli_data(char *cli_buff)
{
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%s", cli_buff);

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

static int cli_parse_line(const char *line, char *words[], int max_words)
{
    int nwords = 0;
    const char *p = line;
    const char *word_start = 0;
    int inquote = 0;

    while (*p)
    {
        if (!(*p == ' '))
        {
            word_start = p;
            break;
        }
        p++;
    }

    while (nwords < max_words - 1)
    {
        if (!*p || *p == inquote || (word_start && !inquote && ((*p == ' ') || *p == '|')))
        {
            if (word_start)
            {
                int len = p - word_start;
                memcpy(words[nwords] = malloc(len + 1), word_start, len);
                words[nwords++][len] = 0;
            }
            if (!*p)
                break;


            if (inquote)
                p++; /* skip over trailing quote */

            inquote = 0;
            word_start = 0;
        }
        else if (*p == '"' || *p == '\'')
        {
            inquote = *p++;
            word_start = p;
        }
        else
        {
            if (!word_start)
            {
                if (*p == '|')
                {
                    if (!(words[nwords++] = "|"))
                        return 0;
                }
                else if (!(*p == ' '))
                    word_start = p;
            }

            p++;
        }
    }
    return nwords;
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
		printf("\n\texecute <TASK> <file_name> <multicast_group_id>");
	printf("\n");
}

void cli_print_multicast_groups(void) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d", SHOW_MULTICAST_GROUPS);

    printf("\nAvailable Multicast groups list:");
    request_cli_data(buffer);
    receive_data();
}

void cli_print_clients(void) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d", SHOW_CLIENTS_ALL);

    printf("\nList of available clients:\n");
    request_cli_data(buffer);
    receive_data();
}

void cli_logging_level(int level)
{
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d", level);

   request_cli_data(buffer);
}

void cli_print_job_details(long long int job_id) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d|%lld", SHOW_JOB_DETAILS, job_id);

    printf("\nJob details for job: %lld\n", job_id);
    request_cli_data(buffer);
    receive_data();

    /*printf("\n\tInput: ");
    printf("\n\tTask: ");
    printf("\n\tStatus: ");*/
    printf("\n");
}

void cli_print_job_result(long long int job_id) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d|%lld", SHOW_JOB_RESULTS, job_id);

    printf("\nResult of Job: %lld\n", job_id);
    request_cli_data(buffer);
    receive_data();
/*
    printf("\n\tStatus: ");
    printf("\n\tResult: ");*/
    printf("\n");
}

void cli_set_repository_address(char *address) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d|%s", SET_REPOSITORY, address);

    printf("\nCentral data repository address updated: %s", address);
    request_cli_data(buffer);
    receive_data();
    printf("\n");
}

void cli_set_job_result_queue_size(int size) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d|%d", SET_JOB_QUEUE_SIZE, size);

    printf("\nJob result queue size updated to: %d", size);
    request_cli_data(buffer);
    receive_data();
    printf("\n");
}

void cli_exec_task(int taskid, char *file, int groupid) {
    char buffer[80];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "%d|%d|%d|%s", EXEC_JOB, taskid, groupid, file);
    
    printf("\nJob submitted to server for execution!\n");
    request_cli_data(buffer);
    receive_data();

/*    printf("\nDetails : %d :  %s : %d", taskid, file, groupid);*/
    printf("\n");
}

void parse_cli(char *cli_string) {
    char *words[CLI_MAX_WORDS] = {0}, *cli_string_trimmed;
    int num_words = 0, i =0;	
    cli_string_trimmed = malloc(strlen(cli_string)+1);
    num_words = cli_parse_line(cli_string, words, sizeof(words)/sizeof(words[0]));
    for(i = 0; i < num_words; i++){
        strcat(cli_string_trimmed, words[i]);
        strcat(cli_string_trimmed, " ");
    }
    cli_string_trimmed[strlen(cli_string_trimmed)-1]= '\0';    

    if (!strcmp(cli_string_trimmed, "show multicast groups")){
        cli_print_multicast_groups();
    } else if(!strcmp(cli_string_trimmed, "show clients all")){
        cli_print_clients();
    } else if(!strcmp(cli_string_trimmed, "logging level error")){
        cli_logging_level(LOGGING_LEVEL_ERROR);
    } else if(!strcmp(cli_string_trimmed, "logging level info")){
        cli_logging_level(LOGGING_LEVEL_INFO);
    } else if(!strcmp(cli_string_trimmed, "logging level debug")){
        cli_logging_level(LOGGING_LEVEL_DEBUG);
    } else if(!strcmp(cli_string_trimmed, "help")) {
        cli_help();
    } else if(!strcmp(cli_string_trimmed, "clear")) {
        cli_clear_screen();
    } else if((!strncmp(cli_string_trimmed, "set job-result-queue size", strlen("set job-result-queue size"))) && (num_words == 4)) {
	cli_set_job_result_queue_size(atoi(words[3]));
    } else if((!strncmp(cli_string_trimmed, "set repository-address", strlen("set repository-address"))) && (num_words == 3)) {
        cli_set_repository_address(words[2]);
    } else if((!strncmp(cli_string_trimmed, "show job-details", strlen("show job-details"))) && (num_words == 3)) {
        cli_print_job_details(strtol(words[2], NULL, 10));
    } else if((!strncmp(cli_string_trimmed, "show job-result", strlen("show job-result"))) && (num_words == 3)) {
   	cli_print_job_result(strtol(words[2], NULL, 10));
    } else if((!strncmp(cli_string_trimmed, "exec", strlen("exec"))) && (num_words == 4)) {	
       int algoType = getAlgoType(words[1]);
       if (algoType == -1)
          return;
       else	
         cli_exec_task(algoType, words[2], atoi(words[3]));	
    } else {
        printf("\nCommand Not found!\n");
        printf("\nEnter \"help\" command to check the list of available commands.");
    }
    //free(cli_string_trimmed);
}

void cli_main(int fd){
    char str[100];
   
    cli_fd = fd;
    make_socket_non_blocking(fd);

    if(!init_cli_interface) {
	if(system("clear") == -1){}
 	printf("\n**************************************************************************************************************");
    	printf("\n************************************* Pyramid CLI Interface **************************************************");
        printf("\n**************************************************************************************************************\n\n");
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

int getAlgoType(char * algoType)
{
  if (strcmp(algoType, "sort") == 0)  return ALGO_SORT;
  if (strcmp(algoType, "max") == 0) return ALGO_MAX;
  
  printf("Unsupported Algorithm \n" );
  return -1;
}
