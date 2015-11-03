#ifndef __GROUPS_H__
#define __GROUPS_H__

#include <arpa/inet.h>

#include "common.h"
#include "jobs.h"

typedef struct client_group client_group;
typedef struct client_info client_info;
typedef struct jobNode job_node;

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
    enum boolean busy; // client's status
    job_node *jn; // jobs information
    pthread_mutex_t lock;  // lock for each client_info
};

extern client_group_head *multicast_groups;
extern client_info_head *client_hash_map;

extern void server_update_job_node(client_info *ci, job_node *jn);
extern uint server_get_max_multicast_groups(void);
extern client_group_head * server_get_client_groups_head(void);
extern client_group_head * server_get_client_gid_head(uint gid);
extern client_info_head * server_get_client_info_head(int);
extern client_info * server_search_client_fd(client_info_head *cih, int cfd);
extern enum boolean server_add_one_client_fd(client_info_head *cih, int cfd, struct sockaddr_in * const sa, uint gids[], uint n_gids);
extern enum boolean server_del_one_client_fd(client_info_head *cih, int cfd);
extern uint server_get_max_hashmap_size(void);

#define DEFAULT_MAX_MULTICAST_GROUPS 1000
#define DEFAULT_MAX_CLIENTS_PER_MULTICAST_GROUP 255
#define DEFAULT_MAX_HASHMAP_SIZE 255

#define FOR_ALL_CLIENT_FDS(p, head, i) 					\
		for ((i) = 0;						\
		     ((head) = (i) < server_get_max_hashmap_size() ?	\
		     &client_hash_map[i] : NULL);			\
		     (i)++)						\
		for ((p) = (head)->h; (p); (p) = (p)->r)

#define FOR_ALL_CLIENT_FDS_IN_HASHMAP_ENTRY(p, head) 			\
		for ((p) = (head)->h; (p); (p) = (p)->r)

#define FOR_ALL_MULTICAST_GROUPS(i) \
		for ((i) = 0; i < server_get_max_multicast_groups(); (i)++)

#define FOR_ALL_GROUP_IDS(p, h) \
                for ((p) = (h); (p); (p) = (p)->n)

#define FOR_THE_MULTICAST_GROUPS(i, n) \
		for ((i) = 0; (i) < (n); (i)++)

#define FOR_ALL_GROUPS_CLIENT_BELONGS_TO(p, h) \
			for ((p) = (h); (p); (p) = (p)->ncg)

#endif /* __GROUPS_H__ */
