#include <stdio.h>
#include <string.h>

void handle_data(Tlv_element);

void receive_data();

void request_cli_data(int);

void cli_clear_screen(void);

void cli_print_multicast_groups(void);

void cli_print_multicast_group_clients(int);

void parse_cli(char *cli_string);

void cli_main(int);
