#ifndef __COMMON_H__
#define __COMMON_H__

#define ONE_KB 1024

#define IPV4_SOCKADDR(sa, f, p, a) {                                    \
                                    (sa).sin_family = (f);      	\
                                    (sa).sin_port = (p);                \
                                    (sa).sin_addr.s_addr = (a);         \
                                   } while (0)

enum boolean {
    FALSE,
    TRUE
};

enum cli_type {
    SHOW_MULTICAST_GROUPS,
    SHOW_CLIENTS_ALL,
    SHOW_JOB_DETAILS,
    SHOW_JOB_RESULTS,
    EXEC_JOB,
    LOGGING_LEVEL_ERROR,
    LOGGING_LEVEL_INFO,
    LOGGING_LEVEL_DEBUG
};

struct thread_args {
    int argc;
    char** argv;
};

static inline void
report_error_and_terminate (char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef unsigned int uint;

#endif /* __COMMON_H__ */