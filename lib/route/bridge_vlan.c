/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * lib/route/bridge_vlan.c		Bridge VLAN database
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <linux/if_bridge.h>

/** @cond SKIP */
static struct nl_cache_ops rtnl_bridge_vlan_ops;
static struct nl_object_ops bridge_vlan_obj_ops;
/** @endcond */

static int bridge_vlan_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return 0;
}

static struct nla_policy br_vlan_db_policy[BRIDGE_VLANDB_ENTRY_MAX + 1] = {
	[BRIDGE_VLANDB_ENTRY_INFO]	= { .type = NLA_BINARY, 
					    .minlen = sizeof(struct bridge_vlan_info),
					    .maxlen = sizeof(struct bridge_vlan_info) },
	[BRIDGE_VLANDB_ENTRY_RANGE]	= { .type = NLA_U16 },
	[BRIDGE_VLANDB_ENTRY_STATE]	= { .type = NLA_U8 },
	[BRIDGE_VLANDB_ENTRY_TUNNEL_INFO] = { .type = NLA_NESTED },
};

static int bridge_vlan_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			  struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	int err = 0;
	return err;
}

static struct nl_object_ops bridge_vlan_obj_ops = {
	.oo_name = "route/bridgevlan",
};

static struct nl_cache_ops bridge_vlan_ops = {
	.co_name = "route/bridgevlan",
	.co_hdrsize = sizeof(struct br_vlan_msg),
	.co_msgtypes = {
			{RTM_NEWVLAN, NL_ACT_NEW, "new"},
			{RTM_DELVLAN, NL_ACT_DEL, "del"},
			{RTM_GETVLAN, NL_ACT_GET, "get"},
			END_OF_MSGTYPES_LIST,
			},
	.co_protocol = NETLINK_ROUTE,
	.co_request_update = bridge_vlan_request_update,
	.co_msg_parser = bridge_vlan_msg_parser,
	.co_obj_ops = &bridge_vlan_obj_ops,
};

static void __init bridge_vlan_init(void)
{
	nl_cache_mngt_register(&bridge_vlan_ops);
}

static void __exit bridge_vlan_exit(void)
{
	nl_cache_mngt_register(&bridge_vlan_ops);
}

/** @} */
