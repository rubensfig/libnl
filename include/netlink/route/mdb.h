/* netlink/route/mdb.c
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 */

#ifndef NETADDR_ADDR_H_
#define NETADDR_ADDR_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_mdb_entry;

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
	__u8  family;
	__u32 ifindex;
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
struct rtnl_mdb *rtnl_mdb_alloc(void);
void rtnl_mdb_put(struct rtnl_mdb *mdb);

int rtnl_mdb_build_add_request(/*struct rtnl_addr *addr, int flags,*/
				/*struct nl_msg **result*/);
int rtnl_mdb_add();

#ifdef __cplusplus
}
#endif

#endif
