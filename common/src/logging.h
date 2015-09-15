enum log_level_t
{
ERROR = 0,
INFO,
DEBUG
};

typedef struct
{
unsigned char level;
}log_level;

log_level logging;

void print_error_level_log(char *);
void print_info_level_log(char *);
void print_debug_level_log(char *);
