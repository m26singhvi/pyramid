#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cli_commands.h"
#include "common.h" // FIXME: take care of relative addressing


void cli_clear_screen(void) {
        system("clear");
}

void cli_help(void) {
  printf("\nList of Available Commands : \n");
  printf("\n\tshow multicast groups");
  printf("\n\tshow clients all");
  printf("\n\thelp");
  printf("\n\tclear");
}

void cli_print_multicast_groups(void) {
        // CLI TYPE = 1
        int cli_type = 1;
        printf("\nAvailable Multicast groups list:\n");
}

void cli_print_clients(void) {
    // CLI_TYPE = 2
    int cli_type = 2;
    printf("\nList of available clients:\n");
    
}

void cli_print_multicast_group_clients(int group_id) {
        printf("\nFollowing clients are available in multicast group %d\n", group_id);
        printf("\n*****Under Development*****\n");
}

void parse_cli(char *cli_string) {
        if (!strcmp(cli_string, "show multicast groups")){
                cli_print_multicast_groups();
        }else if(!strcmp(cli_string, "show clients all")){
                cli_print_clients();
        }else if(!strcmp(cli_string, "help")) {
                cli_help();
        }else if(!strcmp(cli_string, "clear")) {
                cli_clear_screen();
        }else {
                printf("\nCommand Not found!\n");
                printf("\nEnter \"help\" command to check the list of available commands.");
        }
}

//To-Do : when server starts, open a terminal and call this function
void cli_main(){
        char str[50];
        printf("\n ************ Pyramid CLI Interface *************\n\n");

        while(true) {
                printf("\nPYRAMID# ");
		gets(str);
                if(!strcmp(str,"exit")){
                        break;
                }else if(!strcmp(str, "")){
			continue;
		}
                parse_cli(str);
                continue;
        }
        printf("\n***** End *****\n");
}

