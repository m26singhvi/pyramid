//#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "groups.h"
#include "tlv.h"
#include "logging.h"

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
sh_display_job_details(long long int job_id)
{
    printf("\nJob Details for Job: %lld", job_id);
}

void
sh_display_job_results(long long int job_id)
{
    printf("\nJob Result for Job: %lld", job_id);
}

long long
sh_allocate_job_id()
{
    long long int next_job_id = 1;
    return next_job_id; 
}

void
sh_execute_job(long long int job_id)
{
    printf("\nExecute Job: %lld", job_id);  
}

void
sh_parse_cmd (int cfd, char *buf, uint len)
{
    char *to = malloc(len + 1);
    int opcode;
    long long int job_id;
    strncpy(to, buf, len);
    // The opcode in buf in not null terminated, add null
    to[len] = '\0';

    opcode = atoi(to);

    switch (opcode) {
    case SHOW_MULTICAST_GROUPS:
	sh_display_all_multicast_groups(cfd);
	break;
    case SHOW_CLIENTS_ALL:
	sh_display_all_clients(cfd);
	break;
    case SHOW_JOB_DETAILS:
	sh_display_job_details(job_id);
	break;
    case SHOW_JOB_RESULTS:
	sh_display_job_results(job_id);
	break;
    case EXEC_JOB:
	job_id = sh_allocate_job_id();
	sh_execute_job(job_id);
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

    free(to);
}
