/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * lib/route/bridge_vlan.c		Bridge VLAN database
 */

#include <netlink-private/netlink.h>
#include <netlink/route/bridge_vlan.h>
#include <netlink/netlink.h>
#include <linux/if_bridge.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define BRIDGE_VLAN_ATTR_IFINDEX         0x000001
#define BRIDGE_VLAN_ATTR_RANGE           0x000002
#define BRIDGE_VLAN_ATTR_FAMILY          0x000004
#define BRIDGE_VLAN_ATTR_VID             0x000008

static struct nl_cache_ops rtnl_bridge_vlan_ops;
static struct nl_object_ops bridge_vlan_obj_ops;
/** @endcond */

static uint64_t bridge_vlan_compare(struct nl_object *_a, struct nl_object *_b,
				  uint64_t attrs, int flags)
{
	struct rtnl_bridge_vlan *a = (struct rtnl_bridge_vlan *) _a;
	struct rtnl_bridge_vlan *b = (struct rtnl_bridge_vlan *) _b;
	uint64_t diff = 0;

#define BRIDGE_VLAN_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, BRIDGE_VLAN_ATTR_##ATTR, a, b, EXPR)
	diff |= BRIDGE_VLAN_DIFF(IFINDEX, a->ifindex != b->ifindex);
	diff |= BRIDGE_VLAN_DIFF(RANGE, a->range != b->range);
#undef BRIDGE_VLAN_DIFF
	return 0;
}

static int bridge_vlan_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_bridge_vlan *dst = nl_object_priv(_dst);
	struct rtnl_bridge_vlan *src = nl_object_priv(_src);

	dst->ifindex = src->ifindex;
	dst->family = src->family;
	dst->state = src->state;
	dst->vlan_id = src->vlan_id;
	dst->range = src->range;

	return 0;
}

static int bridge_vlan_update(struct nl_object *old_obj, struct nl_object *new_obj)
{
	struct rtnl_bridge_vlan *old = (struct rtnl_bridge_vlan *) old_obj;
	struct rtnl_bridge_vlan *new = (struct rtnl_bridge_vlan *) new_obj;

	return NLE_SUCCESS;
}

static void br_vlan_dump_line(struct nl_object *_obj, struct nl_dump_params *p)
{
	struct rtnl_bridge_vlan *obj = (struct rtnl_bridge_vlan*) obj;

	nl_dump(p, "Ifindex=%d\n", obj->ifindex);
	nl_dump(p, "VLAN=%d\n", obj->vlan_id);
	nl_dump(p, "State=%d\n", obj->state);

	if (obj->range)
		nl_dump(p, "RANGE=%d\n", obj->range);
}

static int bridge_vlan_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	int err;
	struct br_vlan_msg gmsg = {
		.family = AF_BRIDGE,
	};

	err = nl_send_simple(sk, RTM_GETVLAN, NLM_F_DUMP, &gmsg, sizeof(gmsg));

	return err >= 0 ? 0 : err;
}

static struct nla_policy br_vlandb_policy[BRIDGE_VLANDB_MAX + 1] = {
	[BRIDGE_VLANDB_ENTRY] = {.type = NLA_NESTED},
};

static struct nla_policy br_vlandb_entry_policy[BRIDGE_VLANDB_ENTRY_MAX + 1] = {
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
	struct nlattr *tb[BRIDGE_VLANDB_MAX + 1], *ttb[BRIDGE_VLANDB_ENTRY_MAX + 1];
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

	err = pp->pp_cb((struct nl_object *) bvlan, pp);
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
		    [NL_DUMP_DETAILS] = br_vlan_dump_line,
		    [NL_DUMP_STATS] = br_vlan_dump_line,
		    },
	.oo_compare = bridge_vlan_compare,
	.oo_clone = bridge_vlan_clone,
	.oo_update = bridge_vlan_update,
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
 * Build a bridge vlan cache including all Bridge VLAN entries currently configured in the kernel.
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

/**
 * @name Add / Modify
 * @{
 */

static int build_bridge_vlan_msg(int cmd, struct br_vlan_msg *hdr,
			  	 struct rtnl_bridge_vlan *link, int flags, struct nl_msg **result)
{
	struct nl_msg *msg;
	msg = nlmsg_alloc_simple(cmd, flags);
	if (!msg)
		return -NLE_NOMEM;

	*result = msg;
	return 0;
}

int rtnl_bridge_vlan_build_change_request(struct rtnl_bridge_vlan *orig,
				   	  struct rtnl_bridge_vlan *changes, int flags,
				   	  struct nl_msg **result)
{
	struct br_vlan_msg bvlan = {
		.family = orig->family,
		.ifindex = orig->ifindex,
	};
	int err, rt;

	build_bridge_vlan_msg(RTM_SETLINK, &bvlan, changes, flags, result);

	return 0;
}

int rtnl_bridge_vlan_change(struct nl_sock *sk, struct rtnl_bridge_vlan *orig,
		     struct rtnl_bridge_vlan *changes, int flags)
{
	struct nl_msg *msg;
	int err;

	err = rtnl_bridge_vlan_build_change_request(orig, changes, flags, &msg);

	BUG_ON(msg->nm_nlh->nlmsg_seq != NL_AUTO_SEQ);
retry:
	err = nl_send_auto_complete(sk, msg);
	if (err < 0)
		goto errout;

	err = wait_for_ack(sk);
	if (err == -NLE_OPNOTSUPP && msg->nm_nlh->nlmsg_type == RTM_NEWLINK) {
		msg->nm_nlh->nlmsg_type = RTM_SETLINK;
		msg->nm_nlh->nlmsg_seq = NL_AUTO_SEQ;
		goto retry;
	}

errout:
	nlmsg_free(msg);
	return err;
}
/** @} */

/**
 * @name Get/ Set
 * @{
 */
struct rtnl_bridge_vlan *rtnl_bridge_vlan_get(struct nl_cache *cache, int ifindex)
{
	struct rtnl_bridge_vlan *bvlan_entry;

	if (cache->c_ops != &rtnl_bridge_vlan_ops)
		return NULL;

	nl_list_for_each_entry(bvlan_entry, &cache->c_items, ce_list) {
		if (bvlan_entry->ifindex == ifindex) {
			nl_object_get((struct nl_object *) bvlan_entry);
			return bvlan_entry;
		}
	}

	return NULL;

}

int rtnl_bridge_vlan_get_ifindex(struct rtnl_bridge_vlan *bvlan)
{
	return bvlan->ifindex;
}

int rtnl_bridge_vlan_set_ifindex(struct rtnl_bridge_vlan *bvlan, int ifindex)
{
	bvlan->ifindex = ifindex;
	return 0;
}

int rtnl_bridge_vlan_get_vlan_id(struct rtnl_bridge_vlan *bvlan)
{
	return bvlan->vlan_id;
}

int rtnl_bridge_vlan_set_vlan_id(struct rtnl_bridge_vlan *bvlan, uint16_t vid)
{
	bvlan->vlan_id = vid;
	return 0;
}

uint8_t rtnl_bridge_vlan_get_state(struct rtnl_bridge_vlan *bvlan)
{
	return bvlan->state;
}

int rtnl_bridge_vlan_set_state(struct rtnl_bridge_vlan *bvlan, uint8_t state)
{
	bvlan->state = state;
	return 0;
}
/** @} */

struct rtnl_bridge_vlan *rtnl_bridge_vlan_alloc(void)
{
	return (struct rtnl_bridge_vlan *) nl_object_alloc(&bridge_vlan_obj_ops);
}


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
