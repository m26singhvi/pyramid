//#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "common.h"
#include "groups.h"
#include "tlv.h"
#include "logging.h"
#include "jobs.h"
//#include "central_server.h"
#include "dynamic_lib_interface.h"
#include "thpool.h"

long long int sh_job_id = 0; 
extern pthread_mutex_t lock_job_id;
extern threadpool g_thpool;
char central_repo_ip[20]; 
int job_queue_size = 10; // Default job queue size set to 10
/*
extern char job_directory[];
extern char username[];
extern char passwd[];
extern char job_timeout;
*/

char job_directory[500];
char username[20];
char passwd[20];
char job_timeout;

typedef struct {
    int task;
    int cfd;
    int group;
    char input_file[256];
}submit_job_t;    

void
sh_send_encoded_data (int fd, char *data, Attribute type)
{
    int len = strlen(data);


//    printf("\nSend_encoded_data");
     while (len > 0) {
 	Buffer buf;
	char payload[1024];
	memset(payload, 0, 1024);

	buf.payload = payload;
	buf.length = 0;
    
	int encoded_len = encode(type, (void *) data, len, &buf);
	int sent = 0;

 //       printf("\n Send Data : fd : %d, payload %s, encoded_len %d, type %d", fd, payload, encoded_len, type);
	if ((sent = send(fd, payload, encoded_len, 0)) == -1) {
	    report_error_and_terminate("Failed to send data");
	} else {
	    len -= sent;
	}
    }
//    printf("\nSent Type : %d", type);
}

int
sh_try_to_send_data (int fd, char *dest, char *src, int tc, int c, Attribute type)
{
    if ((tc + c + 1) >= ONE_KB) {
	sh_send_encoded_data(fd, dest, type);
	tc = 0;
    }

    c = sprintf(dest + tc, "%s", src);

    return tc + c;
}

static void
sh_display_all_multicast_groups (int cfd)
{
    uint at;
    client_group *cg;
    client_group_head * mg = server_get_client_groups_head();
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    FOR_ALL_MULTICAST_GROUPS(at) {
	FOR_ALL_GROUP_IDS(cg, mg[at].h) {
	    c = snprintf(format_buffer, ONE_KB,
//				"gid %u:\ncfd = %d, cip = %0x, cp = %0x\n",
//				cg->gid, cg->ci->cfd, cg->ci->cip, cg->ci->cp);
				"\ngid %u : %d ", cg->gid, cg->ci->cfd);
	    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
					CLI_DATA);
	}
    }

    if (tc) {
	sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }
}

static void
sh_display_all_clients (int cfd)
{
    client_info *ci;
    client_info_head *cih;
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;
    uint i = 0;

     
    FOR_ALL_CLIENT_FDS (ci, cih, i) {
	c = snprintf(format_buffer, ONE_KB, "%d ", ci->cfd);
	tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
					CLI_DATA);
    }

    if (tc) {
	sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }
}

void 
sh_display_job_details(int cfd, long long int job_id)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    c = snprintf(format_buffer, ONE_KB, "\nJob Details for Job: %lld, cdf %d\n", job_id, cfd);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }


//    printf("\nJob Details for Job: %lld, cdf %d\n", job_id, cfd);
}

void
sh_display_job_results(int cfd, long long int job_id)
{
   char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

     /* Get Job result for given job_id */
    JobNode *jobNode = getJobNode(job_id);
  
    if (jobNode == NULL)
    {
  	 c = snprintf(format_buffer, ONE_KB, "\nInvalid Job Id ");
    } else if(jobNode->numClients) {
        c = snprintf(format_buffer, ONE_KB, "\nJob %lld, Result : Job still in Progress..", job_id);
    } else {
	c = snprintf(format_buffer, ONE_KB, "\nJob %lld, Result : %lld", job_id, jobNode->job.result);	
    }
 
 
//    c = snprintf(format_buffer, ONE_KB, "\nJob %lld, status : Done", job_id);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }


//    printf("\nJob Result for Job: %lld, cfd %d\n", job_id, cfd);
}

long long int
sh_allocate_job_id()
{
   pthread_mutex_lock(&lock_job_id);
   sh_job_id++;
   pthread_mutex_unlock(&lock_job_id); 
   return sh_job_id;
}
void
sh_send_job_failure_to_cli(int cli_fd, long long int job_id)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

//    c = snprintf(format_buffer, ONE_KB, "Failed to execute Job %lld!!!\n\tThis might be due to wront file path or unavailability of resources. ", job_id);
c = snprintf(format_buffer, ONE_KB, "%d", ALGO_ERROR);
    tc = sh_try_to_send_data(cli_fd, storage_buffer, format_buffer, tc, c, ALGO_ERROR);
/*    c = snprintf(format_buffer, ONE_KB, "\tThis might be due to wront file path or unavailability of resources. ");
    tc = sh_try_to_send_data(cli_fd, storage_buffer, format_buffer, tc, c,
                                        ALGO_ERROR);
*/
    if (tc) {
        sh_send_encoded_data(cli_fd, storage_buffer, ALGO_ERROR);
    }
    printf("\nFailed to execute Job %lld !!!", job_id);

}

void
sh_send_job_result_to_cli(int client_fd, Tlv tlv)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;
    printf("%s\n", __func__);
//    printf("\nReady to send job Result to cli: %s at fd %d, tlv.type %d", tlv.value, client_fd, tlv.type);
    c = snprintf(format_buffer, ONE_KB, "[%s]", tlv.value);
    tc = sh_try_to_send_data(client_fd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(client_fd, storage_buffer, CLI_DATA);
    }

//    sh_send_encoded_data(client_fd, data, CLI_DATA);
//    printf("\nJob result sent to CLI");
}


void
sh_execute_job(int cfd, long long int job_id, int task, int multicast_groupid, char *file)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    c = snprintf(format_buffer, ONE_KB, "\t Job   : %lld \n\t task  :%d \n\t group :%d \n\t file  : %s \n", job_id,  task, multicast_groupid, file);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        task);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, task);
    }


//    printf("\nExecute Job: %lld, cfd %d, task %d, group %d, file %s \n", job_id, cfd, task, multicast_groupid, file);  
}

void
sh_set_repository(int cfd, char *repository, char *path, char *user, char *pwd)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;
    enum boolean set = TRUE;

    if ((strncpy(central_repo_ip, repository, 20) == NULL) ||
	 (strncpy(job_directory, path, 500) == NULL) ||
	 (strncpy(username, user, 20) == NULL) || 
	 (strncpy(passwd, pwd, 20) == NULL)) {
	set = FALSE;
    }

    c = snprintf(format_buffer, ONE_KB, "\nCentral repository %sset to %s ",
			set ? "" : "un", central_repo_ip);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }

//   printf("\ncfd - %d, repository %s\n", cfd, repository);
}

void
sh_set_job_timeout(int cfd, int timeout)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;
    enum boolean set = TRUE;

    job_timeout = timeout;
    set = FALSE;

    c = snprintf(format_buffer, ONE_KB, "\nJob timeout %sset to %d ",
                        set ? "" : "un", job_timeout);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }

//   printf("\ncfd - %d, repository %s\n", cfd, repository);
}

void 
sh_set_job_queue_size(int cfd, int size)
{
   job_queue_size = size;
   char storage_buffer[ONE_KB];
   char format_buffer[ONE_KB];
   int tc = 0;
   int c = 0;

    c = snprintf(format_buffer, ONE_KB, "\nJob Queue size set to %d ", job_queue_size);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }

//   printf("\ncfd - %d, queue size %d\n", cfd, size);
}

void *submit_job(void *arg)
{
    submit_job_t *submit_job = (submit_job_t*)arg;
    int cfd = submit_job->cfd;
    int task = submit_job->task;
    int group = submit_job->group;
    char *input_file = &submit_job->input_file[0];
    long long int job_id = sh_allocate_job_id();
    sh_execute_job(cfd, job_id, task, group, input_file);
    if(!initJob(group, job_id, task, input_file) == FAILURE)
    {
        sh_send_job_failure_to_cli(cfd, job_id);
    }else {
        sh_display_job_results(cfd, job_id);
    }
    return NULL;
}


void
sh_parse_cmd (int cfd, char *buff)
{
    int opcode;
    long long int cli_job_id = 0;
    char buf[100];
    char *repo = "";
    char *path = "";
    char *user = "";
    char *pwd = "";
    int queue_size = 0;
    const char delim[2] = "|";
    int job_timeout = 10;
    strcpy(buf, buff);

    switch (atoi(strtok(buf, delim))) {
    case SHOW_MULTICAST_GROUPS:
	sh_display_all_multicast_groups(cfd);
	break;
    case SHOW_CLIENTS_ALL:
	sh_display_all_clients(cfd);
	break;
    case SHOW_JOB_DETAILS:
        cli_job_id = strtol((strtok(NULL, delim)), NULL, 10);
	sh_display_job_details(cfd, cli_job_id);
	break;
    case SHOW_JOB_RESULTS:
        cli_job_id = strtol((strtok(NULL, delim)), NULL, 10);
	sh_display_job_results(cfd, cli_job_id);
	break;
    case SET_REPOSITORY:
        repo = strtok(NULL, delim);
	path = strtok(NULL, delim);
	user = strtok(NULL, delim);
	pwd = strtok(NULL, delim);
        sh_set_repository(cfd, repo, path, user, pwd);
	break;
    case SET_JOB_TIMEOUT:
	job_timeout = atoi(strtok(NULL, delim));
	sh_set_job_timeout(cfd, job_timeout); 
	break;
    case SET_JOB_QUEUE_SIZE:
        queue_size = atoi(strtok(NULL, delim));
        sh_set_job_queue_size(cfd, queue_size);
        break;
    case EXEC_JOB:
    {
        submit_job_t job_s;
        job_s.cfd = cfd;
        job_s.task = atoi(strtok(NULL, delim));
        job_s.group = atoi(strtok(NULL, delim));
        char *input_file;
        input_file = strtok(NULL, delim);
        strcpy(job_s.input_file, input_file);
        thpool_add_work(g_thpool, submit_job, (void *)&job_s);
    }   
       break;
    case LOGGING_LEVEL_ERROR:
        //logging.level = ERROR;
        break;
    case LOGGING_LEVEL_INFO:
        //logging.level = INFO;
        break;
    case LOGGING_LEVEL_DEBUG:
        //logging.level = DEBUG;
        break;
    default:
	printf("\nInvalid Opcode %d\n", opcode);
	break;
    }

    sh_send_encoded_data(cfd, buf, GOOD_BYE);

}
