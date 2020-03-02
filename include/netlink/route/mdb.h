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

struct rtnl_mdb *rtnl_mdb_alloc(void);
void rtnl_mdb_put(struct rtnl_mdb *mdb);

int rtnl_mdb_build_add_request(/*struct rtnl_addr *addr, int flags,*/
				/*struct nl_msg **result*/);
int rtnl_mdb_add();

#ifdef __cplusplus
}
#endif

#endif
