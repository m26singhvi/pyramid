#include <stdio.h>

#include "logging.h"

void
logging_msg_print (enum logging_level level, const char *str)
{
	printf("%d %s\n", level, str);
}
