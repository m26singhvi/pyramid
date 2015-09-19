#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#include "common.h"

#define DEBUG_ALL_BOOLEAN TRUE

#define debug_msg_print(flag, fs, ...) 					\
    if (flag)								\
	printf(fs, ##__VA_ARGS__)

#endif /* __DEBUG_H__ */
