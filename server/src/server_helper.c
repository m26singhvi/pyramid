//#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "groups.h"
#include "tlv.h"
#include "logging.h"

long long int sh_job_id = 0;
char *central_repo = "";
int job_queue_size = 10; // Default job queue size set to 10

static void
sh_send_encoded_data (int fd, char *data, Attribute type)
{
    int len = strlen(data);

     while (len > 0) {
 	Buffer buf;
	char payload[1024];
	memset(payload, 0, 1024);

	buf.payload = payload;
	buf.length = 0;

	int encoded_len = encode(type, (void *) data, len, &buf);
	int sent = 0;

	if ((sent = send(fd, payload, encoded_len, 0)) == -1) {
	    report_error_and_terminate("Failed to send data");
	} else {
	    len -= sent;
	}
    }
}

static inline int
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
				"gid %u:\ncfd = %d, cip = %0x, cp = %0x\n",
				cg->gid, cg->ci->cfd, cg->ci->cip, cg->ci->cp);
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
//    printf("\nDisplaying all clients");
    client_info *ci;
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    FOR_ALL_CLIENT_FDS (ci, server_get_client_info_head(cfd)) {
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


    printf("\nJob Details for Job: %lld, cdf %d\n", job_id, cfd);
}

void
sh_display_job_results(int cfd, long long int job_id)
{
   char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    c = snprintf(format_buffer, ONE_KB, "\nJob Result for Job: %lld, cdf %d\n", job_id, cfd);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }


    printf("\nJob Result for Job: %lld, cfd %d\n", job_id, cfd);
}

void
sh_allocate_job_id()
{
    sh_job_id++;
}

void
sh_execute_job(int cfd, long long int job_id, int task, int multicast_groupid, char *file)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    c = snprintf(format_buffer, ONE_KB, "\nRequest received to execute: \n\t Job: %lld, cfd %d, task %d, group %d, file %s \n", job_id, cfd, task, multicast_groupid, file);
    tc = sh_try_to_send_data(cfd, storage_buffer, format_buffer, tc, c,
                                        CLI_DATA);

    if (tc) {
        sh_send_encoded_data(cfd, storage_buffer, CLI_DATA);
    }


    printf("\nExecute Job: %lld, cfd %d, task %d, group %d, file %s \n", job_id, cfd, task, multicast_groupid, file);  
}

void
sh_set_repository(int cfd, char *repository)
{
    char storage_buffer[ONE_KB];
    char format_buffer[ONE_KB];
    int tc = 0;
    int c = 0;

    central_repo = repository;
    c = snprintf(format_buffer, ONE_KB, "\nCentral repository set to %s ", central_repo);
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
void
sh_parse_cmd (int cfd, char *buff)
{
    int opcode;
    long long int job_id;
    char buf[100];
    char *repo = "";
    int queue_size = 0;
    const char delim[2] = "|";
    strcpy(buf, buff);

    switch (atoi(strtok(buf, delim))) {
    case SHOW_MULTICAST_GROUPS:
	sh_display_all_multicast_groups(cfd);
	break;
    case SHOW_CLIENTS_ALL:
	sh_display_all_clients(cfd);
	break;
    case SHOW_JOB_DETAILS:
        job_id = strtol((strtok(NULL, delim)), NULL, 10);
	sh_display_job_details(cfd, job_id);
	break;
    case SHOW_JOB_RESULTS:
        job_id = strtol((strtok(NULL, delim)), NULL, 10);
	sh_display_job_results(cfd, job_id);
	break;
    case SET_REPOSITORY:
        repo = strtok(NULL, delim);
        sh_set_repository(cfd, repo);
	break;
    case SET_JOB_QUEUE_SIZE:
        queue_size = atoi(strtok(NULL, delim));
        sh_set_job_queue_size(cfd, queue_size);
        break;
    case EXEC_JOB:
	sh_allocate_job_id();
        int task;
        task  = atoi(strtok(NULL, delim));
        int group;
        group = atoi(strtok(NULL, delim));
        char *input_file;
        input_file = strtok(NULL, delim);
        sh_execute_job(cfd, sh_job_id, task, group, input_file);
	break;
    case LOGGING_LEVEL_ERROR:
        logging.level = ERROR;
        break;
    case LOGGING_LEVEL_INFO:
        logging.level = INFO;
        break;
    case LOGGING_LEVEL_DEBUG:
        logging.level = DEBUG;
        break;
    default:
	printf("Invalid Opcode %d\n", opcode);
	break;
    }

    sleep(1);
    sh_send_encoded_data(cfd, buf, GOOD_BYE);

}
