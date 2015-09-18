#ifndef __SERVER_H__
#define __SERVER_H__

#include "tlv.h"

#define BACKLOG (1000 * 250)

void* server (void *);
void receive_data (int );
void handle_data(int, Tlv);
void *cli_request_handler(void*);

#endif /* __SERVER_H__ */
