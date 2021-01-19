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

	struct rtnl_bvlan_entry *rtnl_bvlan_entry_alloc(void);
	void rtnl_bridge_vlan_add_entry(struct rtnl_bridge_vlan *bvlan, struct rtnl_bvlan_entry *entry);
  void rtnl_bridge_vlan_foreach_entry(struct rtnl_bridge_vlan *obj,
      void (*cb)(struct rtnl_bvlan_entry *, void *), void *arg);
  struct rtnl_bvlan_entry *rtnl_bridge_vlan_get_entry_head(struct rtnl_bridge_vlan *obj);
  int rtnl_bridge_vlan_entry_get_vlan_id(struct rtnl_bvlan_entry *bvlan);
  int rtnl_bridge_vlan_entry_set_vlan_id(struct rtnl_bvlan_entry *bvlan, uint16_t vid);
  uint8_t rtnl_bridge_vlan_entry_get_state(struct rtnl_bvlan_entry *bvlan);
  int rtnl_bridge_vlan_entry_set_state(struct rtnl_bvlan_entry *bvlan, uint8_t state);
#ifdef __cplusplus
}
#endif
#endif
