#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "tlv.h"


typedef unsigned int uint;
typedef struct client_group client_group;
typedef struct client_info client_info;

typedef struct client_group_head {
    client_group *h;
    uint tc; // total number of clients in this group
} client_group_head;

struct client_group {
    client_group *p; // previous node
    client_group *n; // next node
    client_info *ci;
    client_group *ncg; // next client_group
    uint gid; // multicast group id
};

typedef struct client_info_head {
    client_info *h;
    uint tc; // total number of clients connected
} client_info_head;

struct client_info {
    client_info *l; // left node
    client_info *r; // right node
    client_group *cg;
    uint tgid; // total number of groups this client is registered to
    int cfd; // client fd
    in_addr_t cip; // client IP
    in_port_t cp; // client port
};




extern uint server_get_max_multicast_groups();
extern client_group_head * server_get_client_group_head(uint gid);
extern client_info_head * server_get_client_info_head();
extern client_group_head multicast_groups[];

#define FOR_ALL_MULTICAST_GROUPS(i) \
                for (i = 0; i < server_get_max_multicast_groups(); i++)

#define FOR_ALL_GROUP_IDS(p, h) \
                for ((p) = (h); (p); (p) = (p)->n)

#define FOR_ALL_CLIENT_FDS(p, head) \
                for ((p) = (head)->h; (p); (p) = (p)->r)

static void
send_data (int fd, char *buffer)
{
     int len = strlen(buffer);
        while (len > 0) {
            Buffer buf;
            char payload[1024];
            memset(payload, 0, 1024);
            buf.payload = payload;
            buf.length = 0;
            int encoded_len = encode(CLI_DATA, (void *)buffer , len, &buf);
 	    int sent = 0;
            if ((sent = send(fd, payload, encoded_len, 0)) == -1) {
                report_error_and_terminate("Failed to send data");
            } else {
                len -= sent;
            }
        }
}

void
cli_display_all_multicast_groups (int cfd)
{
    uint at;
    client_group *p;
    char buffer[ONE_KB];
    FOR_ALL_MULTICAST_GROUPS(at) {
	FOR_ALL_GROUP_IDS(p, multicast_groups[at].h) {
	    printf("gid %u:\ncfd = %d, cip = %0x, cp = %0x\n",
		p->gid, p->ci->cfd, p->ci->cip, p->ci->cp);
	    sprintf(buffer, "gid %u:\ncfd = %d, cip = %0x, cp = %0x\n",
		p->gid, p->ci->cfd, p->ci->cip, p->ci->cp);
	    send_data(cfd, buffer);
	}
    }	
}

void
cli_display_all_clients (int cfd)
{
    client_info *ci;
    char buffer[ONE_KB];
    FOR_ALL_CLIENT_FDS (ci, server_get_client_info_head()) {
	sprintf(buffer, "%d\n", ci->cfd);
	printf("%d\n", ci->cfd);
	send_data(cfd, buffer);
    }
}

void
cli_parser (int cfd, char *buf, uint len)
{
    char *to = malloc(len + 1);
    int opcode;
    strncpy(to, buf, len);
    to[len] = '\0';

    opcode = atoi(to);

    switch (opcode) {
    case SHOW_MULTICAST_GROUPS:
	cli_display_all_multicast_groups(cfd);
	break;
    case SHOW_CLIENTS_ALL:
	cli_display_all_clients(cfd);
	break;
    }

    free(to);
}
