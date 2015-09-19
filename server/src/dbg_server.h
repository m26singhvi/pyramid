#ifndef __DBG_SERVER_H__
#define __DBG_SERVER_H__

#include "debug.h"

#define DBG_SERVER_ALL_BOOLEAN DEBUG_ALL_BOOLEAN

enum dbg_server {
    DBG_SERVER_GENERAL,
    DBG_SERVER_MEMORY,
    DBG_SERVER_LAST /* should always be in last */
};

enum boolean dbg_server_arr[DBG_SERVER_LAST] = {
    [DBG_SERVER_GENERAL] DBG_SERVER_ALL_BOOLEAN,
    [DBG_SERVER_MEMORY]  DBG_SERVER_ALL_BOOLEAN
};

#define dbg_server_memory(fs, ...)						\
    debug_msg_print(dbg_server_arr[DBG_SERVER_MEMORY], fs, ##__VA_ARGS__)

#endif /* __DBG_SERVER_H__ */
