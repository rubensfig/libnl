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

static void br_vlan_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
	nl_dump(p, "hello\n", 1);
}

static int bridge_vlan_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return nl_rtgen_request(sk, RTM_GETVLAN, AF_BRIDGE, NLM_F_DUMP);
}

static struct nla_policy br_vlandb_policy[BRIDGE_VLANDB_MAX 1] = {
	[BRIDGE_VLANDB_ENTRY] = {.type = NLA_NESTED},
};

static struct nla_policy br_vlandb_entry_policy[BRIDGE_VLANDB_ENTRY_MAX 1] = {
	[BRIDGE_VLANDB_ENTRY_INFO]	= { .type = NLA_BINARY, 
					    .minlen = sizeof(struct bridge_vlan_info),
					    .maxlen = sizeof(struct bridge_vlan_info) },
	[BRIDGE_VLANDB_ENTRY_RANGE]	= { .type = NLA_U16 },
	[BRIDGE_VLANDB_ENTRY_STATE]	= { .type = NLA_U8 },
	[BRIDGE_VLANDB_ENTRY_TUNNEL_INFO] = { .type = NLA_NESTED },
};

struct rtnl_bridge_vlan *rtnl_bridge_vlan_alloc(void)
{
	return (struct rtnl_bridge_vlan *) nl_object_alloc(&bridge_vlan_obj_ops);
}

static int bridge_vlan_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			  struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	int err = 0;
	struct nlattr *tb[BRIDGE_VLANDB_MAX 1], *ttb[BRIDGE_VLANDB_ENTRY_MAX 1];
	uint8_t state = 0;
	uint16_t range = 0;
	struct bridge_vlan_info *bvlan_info;
	struct rtnl_bridge_vlan *bvlan = rtnl_bridge_vlan_alloc();

	err = nlmsg_parse(nlh, sizeof(struct br_vlan_msg), tb, BRIDGE_VLANDB_MAX,
			  br_vlandb_policy);

	if (err < 0)
		goto errout;

	if (!tb[BRIDGE_VLANDB_ENTRY])
	{
		err = -NLE_NOATTR;
		goto errout;
	}

	nla_parse_nested(ttb, BRIDGE_VLANDB_ENTRY_MAX, tb[BRIDGE_VLANDB_ENTRY], br_vlandb_entry_policy);

	if (ttb[BRIDGE_VLANDB_ENTRY_INFO])
		bvlan_info = nla_data(ttb[BRIDGE_VLANDB_ENTRY_INFO]);

	if (ttb[BRIDGE_VLANDB_ENTRY_STATE])
		state = nla_get_u8(ttb[BRIDGE_VLANDB_ENTRY_STATE]);

	if (ttb[BRIDGE_VLANDB_ENTRY_RANGE])
		range = nla_get_u16(ttb[BRIDGE_VLANDB_ENTRY_RANGE]);

	bvlan->state = state;
	bvlan->flags = bvlan_info->flags;
	bvlan->vlan_id = bvlan_info->vid;
	bvlan->range = range;

	return err;
errout:
	rtnl_bridge_vlan_put(bvlan);
	return err;
}

static struct nl_af_group br_vlan_groups[] = {
	{AF_BRIDGE, RTNLGRP_BRVLAN},
	{END_OF_GROUP_LIST},
};

static struct nl_object_ops bridge_vlan_obj_ops = {
	.oo_name = "route/bridgevlan",
	.oo_size = sizeof(struct rtnl_bridge_vlan),
	.oo_dump = {
		    [NL_DUMP_LINE] = br_vlan_dump_line,
		    },
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
	.co_groups = br_vlan_groups,
	.co_request_update = bridge_vlan_request_update,
	.co_msg_parser = bridge_vlan_msg_parser,
	.co_obj_ops = &bridge_vlan_obj_ops,
};

/**
 * @name Cache Management
 * @{
 */
int rtnl_bridge_vlan_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&bridge_vlan_ops, sk, result);
}

/**
 * Build a neighbour cache including all Bridge VLAN entries currently configured in the kernel.
 * @arg sock		Netlink socket.
 * @arg result		Pointer to store resulting cache.
 * @arg flags		Flags to apply to cache before filling
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_bridge_vlan_alloc_cache_flags(struct nl_sock *sock, struct nl_cache **result,
			       unsigned int flags)
{
	struct nl_cache *cache;
	int err;

	cache = nl_cache_alloc(&bridge_vlan_ops);
	if (!cache)
		return -NLE_NOMEM;

	nl_cache_set_flags(cache, flags);

	if (sock && (err = nl_cache_refill(sock, cache)) < 0) {
		nl_cache_free(cache);
		return err;
	}

	*result = cache;
	return 0;
}
/** @} */

void rtnl_bridge_vlan_put(struct rtnl_bridge_vlan *bvlan)
{
	nl_object_put((struct nl_object *) bvlan);

}

static void __init bridge_vlan_init(void)
{
	nl_cache_mngt_register(&bridge_vlan_ops);
}

static void __exit bridge_vlan_exit(void)
{
	nl_cache_mngt_register(&bridge_vlan_ops);
}

/** @} */
