#include <stdio.h>
#include <string.h>
#include "tlv.h"

unsigned int handle_data(Tlv_element);

void receive_data();

void request_cli_data(char *);

void cli_clear_screen(void);

void cli_print_multicast_groups(void);

void cli_print_multicast_group_clients(int);

void cli_logging_level(int);

void parse_cli(char *cli_string);

void cli_main(int);
