#include <stdio.h>
#include <string.h>

char* request_thread(int);

void cli_clear_screen(void);

void cli_print_multicast_groups(void);

void cli_print_multicast_group_clients(int);

void parse_cli(char *cli_string);

void cli_main();
