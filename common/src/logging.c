#include <stdio.h>




#include <time.h>

#include "logging.h"

#define MAX_TIME_LENGTH 500

const char *logging_time_fmt = "year=%d month=%d day=%d hour=%d min=%d sec=%d";
const char *logging_date_and_time_fmt = "%s-%d %d:%d:%d:";

const char *month_num2alpha[] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP","OCT", "NOV", "DEC"
};

const char *level_num2alpah[] = {
    [LOGGING_LEVEL_ALERTS]        = "ALERTS",
    [LOGGING_LEVEL_CRITICAL]      = "CRITICAL",
    [LOGGING_LEVEL_ERRORS]        = "ERRORS",
    [LOGGING_LEVEL_NOTIFICATIONS] = "NOTIFICATIONS",
    [LOGGING_LEVEL_INFORMATIONAL] = "INFORMATIONAL"
};

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

static inline void
logging_get_date (char buf[])
{
    (void)buf;
}

static inline void
logging_get_date_and_time (char buf[])
{
    struct tm tm = get_system_time();

    snprintf(buf, MAX_TIME_LENGTH, logging_date_and_time_fmt,
	     month_num2alpha[tm.tm_mon], tm.tm_mday,
	     tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void
logging_message (enum logging_level level)
{
    char buf[MAX_TIME_LENGTH] = {0};

    logging_get_date_and_time(buf);
    printf("%s %d-%s: ", buf, level, level_num2alpah[level]);
}

#if 0
int
main ()
{
        int i = 0;
	while(++i <= 1000)
	logging_msg_print(0, "hello");
	logging_notifications("Hello");
	logging_alerts("hi %d", 90);
	return 0;
}
#endif
