/* SPDX-License-Identifier: LGPL-2.1-only */

#ifndef NETLINK_B_VLAN_H_
#define NETLINK_B_VLAN_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>

#ifdef __cplusplus
extern "C" {
#endif
	struct rtnl_bridge_vlan *rtnl_bridge_vlan_alloc(void);
	struct rtnl_bridge_vlan *rtnl_bridge_vlan_get(struct nl_cache *cache, int ifindex);
	void rtnl_bridge_vlan_put(struct rtnl_bridge_vlan *bvlan);
	int rtnl_bridge_vlan_alloc_cache(struct nl_sock *sk,
					 struct nl_cache **result);
	int rtnl_bridge_vlan_alloc_cache_flags(struct nl_sock *sock,
					       struct nl_cache **result,
					       unsigned int flags);
	int rtnl_bridge_vlan_change(struct nl_sock *sk, struct rtnl_bridge_vlan *orig,
			     struct rtnl_bridge_vlan *changes, int flags);
	int rtnl_bridge_vlan_build_change_request(struct rtnl_bridge_vlan *orig,
						  struct rtnl_bridge_vlan *changes, int flags,
						  struct nl_msg **result);
	int rtnl_bridge_vlan_get_ifindex(struct rtnl_bridge_vlan *bvlan);
	int rtnl_bridge_vlan_set_ifindex(struct rtnl_bridge_vlan *bvlan, int ifindex);

#ifdef __cplusplus
}
#endif
#endif
