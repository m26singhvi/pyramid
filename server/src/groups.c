#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

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

/*
 * This is a client multicast group array
 */
const uint max_multicast_groups = 1000;
uint max_clients_per_multicast_group = 250;
client_group_head multicast_groups[1000];
client_info_head ci_head = {NULL, 0};

/*
 * This is the header to the all the client information list
 */
client_info_head ci_list_head;

uint
server_get_max_multicast_groups ()
{
    return max_multicast_groups;
}

static inline uint
server_get_max_clients_per_multicast_group ()
{
    return max_clients_per_multicast_group;
}

static inline int
server_is_multicast_group_valid (uint gid)
{
    return (1 <= gid) && (gid <= server_get_max_multicast_groups());
}

client_group_head *
server_get_client_group_head (uint gid)
{
    if (server_is_multicast_group_valid(gid)) {
	return &multicast_groups[gid - 1];
    } else {
	return NULL;
    }
}

client_info_head *
server_get_client_info_head ()
{
    return &ci_list_head;
}


#define FOR_ALL_CLIENT_FDS(p, head) \
		for ((p) = (head)->h; (p); (p) = (p)->r)

#define FOR_ALL_MULTICAST_GROUPS(i) \
		for (i = 0; i < server_get_max_multicast_groups(); i++)

#define FOR_ALL_GROUP_IDS(p, h) \
                for ((p) = (h); (p); (p) = (p)->n)

#define FOR_THE_MULTICAST_GROUPS(i, n) \
		for ((i) = 0; (i) < (n); (i)++)

#define FOR_ALL_GROUPS_CLIENT_BELONGS_TO(p, h) \
			for ((p) = (h); (p); (p) = (p)->ncg)

client_info *
server_search_client_fd (client_info_head *head, int cfd)
{
    client_info *ci;
    FOR_ALL_CLIENT_FDS(ci, head) {
	if (cfd == ci->cfd) {
	    return ci;
	}
    }
    return NULL;
}

static inline client_info *
server_alloc_client_info_node (void)
{
    return malloc(sizeof(client_info));
}

static inline void
server_destroy_client_info_node (client_info *ci)
{
    free((void *) ci);
}

static inline void
server_initialize_client_info_node (client_info_head *cih,
				    client_info *nci,
				    client_group *cg,
				    uint n_gids,
				    int cfd,
				    in_addr_t cip,
				    in_port_t cp)
{
    nci->l = NULL;
    nci->r = cih->h;
    nci->cg = cg;
    nci->tgid = n_gids;
    nci->cfd = cfd;
    nci->cip = cip;
    nci->cp = cp;
}

static inline client_group *
server_alloc_client_group_node (void)
{
    return malloc(sizeof(client_group));
}

static inline void
server_destroy_client_group_node (client_group *ci)
{
    free((void *) ci);
}

static inline void
server_initialize_client_group_node (client_group_head *cgh,
				     client_group *ncg,
				     client_info *ci,
				     client_group *cg,
				     uint gid)
{
    ncg->p = NULL;
    ncg->n = cgh->h;
    ncg->ci = ci;
    ncg->ncg = cg;
    ncg->gid = gid;
}

/*
 * Other APIs rely on the fact that the gids added are valid.
 * This add fucntion should make sure that only valid gids gets added.
 */
static inline client_group *
server_add_client_to_one_group (client_info * ci, uint gid, client_group *cg)
{
    client_group_head *cgh = server_get_client_group_head(gid);
    client_group *ncg = server_alloc_client_group_node();
    uint mc = server_get_max_clients_per_multicast_group();

    if ((ncg == NULL) || (cgh == NULL)) {
	return NULL;
    }

    if (cgh->tc >= mc) {
	// below print should be added as log
	printf("Group add request failed for gid %u"
	       "as max client per multicast group %u is reached\n", gid, mc);
	return NULL;
    }
    server_initialize_client_group_node(cgh, ncg, ci, cg, gid);

    if (cgh->h) {
	cgh->h->p = ncg;
    }

    cgh->h = ncg;
    cgh->tc++;
    
    return ncg;
}

static inline void
server_del_client_from_groups (client_group *cg)
{
    client_group_head *cgh = NULL;
    client_group *me = cg;

    while (cg) {
	me = cg;
	cg = cg->ncg;
	cgh = server_get_client_group_head(me->gid);

	if (me->p) {
	    me->p->n = me->n;
	} else {
	    cgh->h = me->n;
	}
	if (me->n) {
	    me->n->p = me->p;
	}
	cgh->tc--;

	server_destroy_client_group_node(me);
    }
}

static inline client_group *
server_add_client_to_groups (client_info *ci, uint gids[], int n_gids)
{
    int at;
    client_group *cg = NULL;
    client_group *cgt = NULL;
    
    FOR_THE_MULTICAST_GROUPS(at, n_gids) {
	cgt = server_add_client_to_one_group(ci, gids[at], cg);
	/*
 	 * If we are unable to create and add client to any one of the
	 * groups, delete it from all the previously added group.
	 */
	if (cgt == NULL) {
	    server_del_client_from_groups(cg);
	    return NULL;
	} else {
	    cg = cgt;
	}
    }

    return cg;
}

static enum boolean
server_add_one_new_client_fd (client_info_head *cih,
			      int cfd,
			      struct sockaddr_in * const sa,
			      uint gids[],
			      int n_gids)
{
    client_info *nci = server_alloc_client_info_node();

    if (nci) {
	client_group * cg = server_add_client_to_groups(nci, gids, n_gids);
	if (cg == NULL) {
	    server_destroy_client_info_node(nci);
	    return FALSE;
	}

	server_initialize_client_info_node(cih, nci, cg, n_gids, cfd,
			ntohl(sa->sin_addr.s_addr), ntohs(sa->sin_port));

	if (cih->h) {
	    cih->h->l = nci;
	}
	
	cih->h = nci;
	cih->tc++;
	return TRUE;
    }

    return FALSE;
}

enum boolean
server_add_one_client_fd (client_info_head *cih,
			  int cfd,
			  struct sockaddr_in * const sa,
			  uint gids[],
			  uint n_gids)
{
    if (cih == NULL) {
	return FALSE;
    }

    if (n_gids > server_get_max_multicast_groups()) {
	printf("Groups exceed max multicast groups");
	return FALSE;
    }
    /*
     * This means the previous client conn is terminated and the
     * fd is realocated.
     */
    if (server_search_client_fd(cih, cfd) != NULL) {
	printf("Add logic to to take care of previous client conn is "
		"terminated and the fd is realocated case.\n");
	// This print should be changed to log as previous conn is terminated
	printf("Replacing previous client fd");
	//return FALSE;
    }

    server_add_one_new_client_fd(cih, cfd, sa, gids, n_gids);

    return TRUE;
}

enum boolean
server_del_one_client_fd (client_info_head *cih,
			  int cfd)
{
    client_info *ci;

    if (cih == NULL) {
	return FALSE;
    }
    
    if ((ci = server_search_client_fd(cih, cfd)) == NULL) {
	printf("Client fd %d does not exist", cfd);
	return FALSE;
    }

    server_del_client_from_groups(ci->cg);

    if (ci->l) {
	ci->l->r = ci->r;
    } else {
	cih->h = ci->r;
    }
    if (ci->r) {
	ci->r->l = ci->l;
    }
    cih->tc--;

    server_destroy_client_info_node(ci);

    return TRUE;
}
