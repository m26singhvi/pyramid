#include <stdio.h>
#include <time.h>

#include "logging.h"

#define MAX_TIME_LENGTH 500

const char *logging_time_fmt = "year=%d month=%d day=%d hour=%d min=%d sec=%d";

struct tm
get_system_time (void)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    return tm;
}

static inline void
logging_get_time (char buf[])
{
    struct tm tm = get_system_time();
    snprintf(buf, MAX_TIME_LENGTH, logging_time_fmt, (tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void
logging_msg_print (enum logging_level level, const char *str)
{
    char buf[MAX_TIME_LENGTH] = {0};
    logging_get_time(buf);
    printf("%d %s %s\n", level, buf, str);
}
