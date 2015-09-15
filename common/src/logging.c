#include <stdio.h>
#include "logging.h"

void print_debug_level_log(char *str)
{
    if (logging.level == DEBUG)
    {
       printf("\n%s",str);
    }
}

void print_info_level_log(char *str)
{
    if (logging.level == INFO)
    {
       printf("\n%s",str);
    }
}

void print_error_level_log(char *str)
{
    if (logging.level == ERROR)
    {
       printf("\n%s",str);
    }
}

