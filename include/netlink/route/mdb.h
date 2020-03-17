/*
 *  netlink/route/mdb.c
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 */

#ifndef NETLINK_MDB_H_
#define NETLINK_MDB_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_mdb_entry;

// DOES NOT BELONG HERE
//
/* Bridge multicast database attributes
 * [MDBA_MDB] = {
 *     [MDBA_MDB_ENTRY] = {
 *         [MDBA_MDB_ENTRY_INFO] {
 *		struct br_mdb_entry
 *		[MDBA_MDB_EATTR attributes]
 *         }
 *     }
 * }
 * [MDBA_ROUTER] = {
 *    [MDBA_ROUTER_PORT] = {
 *        u32 ifindex
 *        [MDBA_ROUTER_PATTR attributes]
 *    }
 * }
 */

enum {
	MDBA_UNSPEC,
	MDBA_MDB,
	MDBA_ROUTER,
	__MDBA_MAX,
};
#define MDBA_MAX (__MDBA_MAX - 1)

enum {
	MDBA_MDB_UNSPEC,
	MDBA_MDB_ENTRY,
	__MDBA_MDB_MAX,
};
#define MDBA_MDB_MAX (__MDBA_MDB_MAX - 1)

enum {
	MDBA_MDB_ENTRY_UNSPEC,
	MDBA_MDB_ENTRY_INFO,
	__MDBA_MDB_ENTRY_MAX,
};
#define MDBA_MDB_ENTRY_MAX (__MDBA_MDB_ENTRY_MAX - 1)

struct br_port_msg {
	uint8_t  family;
	uint32_t ifindex;
};

struct br_mdb_entry {
	__u32 ifindex;
#define MDB_TEMPORARY 0
#define MDB_PERMANENT 1
	__u8 state;
#define MDB_FLAGS_OFFLOAD	(1 << 0)
	__u8 flags;
	__u16 vid;
	struct {
		union {
			__be32	ip4;
			struct in6_addr ip6;
		} u;
		__be16		proto;
	} addr;
};

enum {
	MDBA_SET_ENTRY_UNSPEC,
	MDBA_SET_ENTRY,
	__MDBA_SET_ENTRY_MAX,
};
#define MDBA_SET_ENTRY_MAX (__MDBA_SET_ENTRY_MAX - 1)
// DOES NOT BELONG HERE

struct rtnl_mdb *rtnl_mdb_alloc(void);
void rtnl_mdb_put(struct rtnl_mdb *mdb);

int rtnl_mdb_alloc_cache(struct nl_sock *sk, struct nl_cache **result);
int rtnl_mdb_alloc_cache_flags(struct nl_sock *sock, struct nl_cache **result, unsigned int flags);
int rtnl_mdb_build_add_request(/*struct rtnl_addr *addr, int flags,*/
				/*struct nl_msg **result*/);
int rtnl_mdb_add();
struct rtnl_mdb_entry *rtnl_mdb_entry_alloc(void);

uint32_t rtnl_mdb_get_ifindex(struct rtnl_mdb *mdb);
uint8_t rtnl_mdb_get_family(struct rtnl_mdb *mdb);

void rtnl_mdb_add_entry(struct rtnl_mdb *mdb, struct rtnl_mdb_entry *_entry);

void rtnl_mdb_foreach_entry(struct rtnl_mdb *_mdb,
                            void (*cb)(struct rtnl_mdb_entry *, void *),
                            void *arg);
//struct nl_list_head * rtnl_mdb_get_entries(struct rtnl_mdb *mdb);
void rtnl_mdb_foreach_entry(struct rtnl_mdb *mdb,
                            void (*cb)(struct rtnl_mdb_entry *, void *),
                            void *arg );

int rtnl_mdb_entry_get_ifindex(struct rtnl_mdb_entry *mdb_entry);
int rtnl_mdb_entry_get_vid(struct rtnl_mdb_entry *mdb_entry);
int rtnl_mdb_entry_get_state(struct rtnl_mdb_entry *mdb_entry);
struct nl_addr * rtnl_mdb_entry_get_addr(struct rtnl_mdb_entry *mdb_entry);
struct nl_addr * rtnl_mdb_entry_get_proto(struct rtnl_mdb_entry *mdb_entry);
#ifdef __cplusplus
}
#endif

#endif
