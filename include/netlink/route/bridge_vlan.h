/* SPDX-License-Identifier: LGPL-2.1-only */

#ifndef NETLINK_B_VLAN_H_
#define NETLINK_B_VLAN_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif
	struct rtnl_bridge_vlan *rtnl_bridge_vlan_alloc(void);
	int rtnl_bridge_vlan_alloc_cache(struct nl_sock *sk,
					 struct nl_cache **result);
	int rtnl_bridge_vlan_alloc_cache_flags(struct nl_sock *sock,
					       struct nl_cache **result,
					       unsigned int flags);
#ifdef __cplusplus
}
#endif
#endif
