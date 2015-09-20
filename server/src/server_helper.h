#ifndef __SERVER_HELPER_H__
#define __SERVER_HELPER_H__

#include "common.h"

extern void sh_parse_cmd(int cfd, char *buf, uint len);

void sh_send_encoded_data (int fd, char *data, Attribute type);

void sh_send_job_failure_to_cli(int cli_fd, long long int job_id);

void sh_send_job_result_to_cli(int client_fd, Tlv tlv);

int sh_try_to_send_data (int fd, char *dest, char *src, int tc, int c, Attribute type);

#endif /* __SERVER_HELPER_H__ */
