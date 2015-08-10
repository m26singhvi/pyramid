#ifndef __SERVER_H__
#define __SERVER_H__

#define BACKLOG (1000 * 250)

void* server (void *);
void receive_data (int );

#endif /* __SERVER_H__ */
