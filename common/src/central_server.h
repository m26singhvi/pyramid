#ifndef __CENTRAL_SERVER_H__
#define __CENTRAL_SERVER_H__

#define MAX_SSH_CMD_SIZE 2048
#define IP_ADDR_LEN 20
#define JOB_DIR_PATH_LEN 500
#define USERNAME_LEN 20
#define PASSWORD_LEN 20

char central_repo_ip[IP_ADDR_LEN];
char job_directory[JOB_DIR_PATH_LEN];
char username[USERNAME_LEN];
char passwd[PASSWORD_LEN];
char job_timeout;

const char * cntrl_srv_get_central_repo_ip(void);
const char * cntrl_srv_get_job_directory(void);
const char * cntrl_srv_get_username(void);
const char * cntrl_srv_get_passwd(void);

#endif /* __CENTRAL_SERVER_H__ */
