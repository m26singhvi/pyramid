#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "tlv.h"

unsigned int handle_data(Tlv);

void receive_data();

bool handleNetworkMessage(char *buf, int inlen);

void request_cli_data(char *);

void cli_clear_screen(void);

void cli_print_multicast_groups(void);

void cli_print_multicast_group_clients(int);

void cli_print_clients(void);

void cli_print_job_details(long long int);

void cli_print_job_result(long long int);

void cli_set_repository_address(char *, char *, char *, char *);

void cli_set_job_result_queue_size(int);

void cli_exec_task(int, char *, int);

void cli_logging_level(int);

void parse_cli(char *cli_string);

void cli_main(int);

int getAlgoType(char *);
