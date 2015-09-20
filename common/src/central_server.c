#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "central_server.h"

#define SUCCESS 0
#define FAILURE 1

char central_repo_ip[IP_ADDR_LEN];

const char *job_directory = "/tmp/cohort/";
const char *username = "repository";
const char *passwd = "ashuvenda";

int
db_server_divide (char *path,
		  unsigned int job_id,
		  unsigned int n)
{
   if (n == 0) {
	printf("Number of parts = 0");
	return FAILURE;
   }

   char sshcmd[MAX_SSH_CMD_SIZE] = {0};
   const char *sshsplitfmt = "sshpass -p%s ssh -o StrictHostKeyChecking=no %s@%s /home/repository/cohort/dos.sh %d %d %s";

   snprintf(sshcmd, MAX_SSH_CMD_SIZE, sshsplitfmt, passwd, username, central_repo_ip, n, job_id, path);
   // make this debug: printf("db_server_divide: %s\n", sshcmd);

   if (system(sshcmd) == 0)
   {
	printf("Split Successful \n");
	return SUCCESS;
   } else {
	return FAILURE;
   }
}

const char *
cntrl_srv_get_job_directory (void)
{
    return job_directory;
}

const char *
cntrl_srv_get_central_repo_ip (void)
{
    return central_repo_ip;
}

const char *
cntrl_srv_get_username (void)
{
    return username;
}

const char *
cntrl_srv_get_passwd (void)
{
    return passwd;
}
