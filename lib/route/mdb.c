/*
 * lib/route/mdb.c		Multicast Database
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2003-2006 Baruch Even <baruch@ev-en.org>,
 *                         Mediatrix Telecom, inc. <ericb@mediatrix.com>
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/route/mdb.h>
#include <netlink/utils.h>

static struct nl_cache_ops rtnl_mdb_ops;
static struct nl_object_ops mdb_obj_ops;
/** @endcond */

static void mdb_constructor(struct nl_object *obj)
{
}

static void mdb_free_data(struct nl_object *obj)
{
  // Cleans the functions
}

// clones the mdb object
static int mdb_clone(struct nl_object *_dst, struct nl_object *_src)
{
#if 0
	struct rtnl_addr *dst = nl_object_priv(_dst);
	struct rtnl_addr *src = nl_object_priv(_src);

	if (src->a_link) {
		nl_object_get(OBJ_CAST(src->a_link));
		dst->a_link = src->a_link;
	}

	if (src->a_peer)
		if (!(dst->a_peer = nl_addr_clone(src->a_peer)))
			return -NLE_NOMEM;
	
	if (src->a_local)
		if (!(dst->a_local = nl_addr_clone(src->a_local)))
			return -NLE_NOMEM;

	if (src->a_bcast)
		if (!(dst->a_bcast = nl_addr_clone(src->a_bcast)))
			return -NLE_NOMEM;

	if (src->a_multicast)
		if (!(dst->a_multicast = nl_addr_clone(src->a_multicast)))
			return -NLE_NOMEM;

	if (src->a_anycast)
		if (!(dst->a_anycast = nl_addr_clone(src->a_anycast)))
			return -NLE_NOMEM;

	return 0;
#endif
}

/*static struct nla_policy mdb_policy[IFA_MAX+1] = {
	[IFA_LABEL]	= { .type = NLA_STRING,
			    .maxlen = IFNAMSIZ },
	[IFA_CACHEINFO]	= { .minlen = sizeof(struct ifa_cacheinfo) },
};*/

static int mdb_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
#if 0
	struct rtnl_addr *addr;
	struct ifaddrmsg *ifa;
	struct nlattr *tb[IFA_MAX+1];
	int err, family;
	struct nl_cache *link_cache;
	struct nl_addr *plen_addr = NULL;

	addr = rtnl_addr_alloc();
	if (!addr)
		return -NLE_NOMEM;

	addr->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(*ifa), tb, IFA_MAX, addr_policy);
	if (err < 0)
		goto errout;

	ifa = nlmsg_data(nlh);
	addr->a_family = family = ifa->ifa_family;
	addr->a_prefixlen = ifa->ifa_prefixlen;
	addr->a_scope = ifa->ifa_scope;
	addr->a_flags = tb[IFA_FLAGS] ? nla_get_u32(tb[IFA_FLAGS]) :
					ifa->ifa_flags;
	addr->a_ifindex = ifa->ifa_index;

	addr->ce_mask = (ADDR_ATTR_FAMILY | ADDR_ATTR_PREFIXLEN |
			 ADDR_ATTR_FLAGS | ADDR_ATTR_SCOPE | ADDR_ATTR_IFINDEX);

	if (tb[IFA_LABEL]) {
		nla_strlcpy(addr->a_label, tb[IFA_LABEL], IFNAMSIZ);
		addr->ce_mask |= ADDR_ATTR_LABEL;
	}

	/* IPv6 only */
	if (tb[IFA_CACHEINFO]) {
		struct ifa_cacheinfo *ca;
		
		ca = nla_data(tb[IFA_CACHEINFO]);
		addr->a_cacheinfo.aci_prefered = ca->ifa_prefered;
		addr->a_cacheinfo.aci_valid = ca->ifa_valid;
		addr->a_cacheinfo.aci_cstamp = ca->cstamp;
		addr->a_cacheinfo.aci_tstamp = ca->tstamp;
		addr->ce_mask |= ADDR_ATTR_CACHEINFO;
	}

	if (family == AF_INET) {
		uint32_t null = 0;

		/* for IPv4/AF_INET, kernel always sets IFA_LOCAL and IFA_ADDRESS, unless it
		 * is effectively 0.0.0.0. */
		if (tb[IFA_LOCAL])
			addr->a_local = nl_addr_alloc_attr(tb[IFA_LOCAL], family);
		else
			addr->a_local = nl_addr_build(family, &null, sizeof (null));
		if (!addr->a_local)
			goto errout_nomem;
		addr->ce_mask |= ADDR_ATTR_LOCAL;

		if (tb[IFA_ADDRESS])
			addr->a_peer = nl_addr_alloc_attr(tb[IFA_ADDRESS], family);
		else
			addr->a_peer = nl_addr_build(family, &null, sizeof (null));
		if (!addr->a_peer)
			goto errout_nomem;

		if (!nl_addr_cmp (addr->a_local, addr->a_peer)) {
			/* having IFA_ADDRESS equal to IFA_LOCAL does not really mean
			 * there is no peer. It means the peer is equal to the local address,
			 * which is the case for "normal" addresses.
			 *
			 * Still, clear the peer and pretend it is unset for backward
			 * compatibility. */
			nl_addr_put(addr->a_peer);
			addr->a_peer = NULL;
		} else
			addr->ce_mask |= ADDR_ATTR_PEER;

		plen_addr = addr->a_local;
	} else {
		if (tb[IFA_LOCAL]) {
			addr->a_local = nl_addr_alloc_attr(tb[IFA_LOCAL], family);
			if (!addr->a_local)
				goto errout_nomem;
			addr->ce_mask |= ADDR_ATTR_LOCAL;
			plen_addr = addr->a_local;
		}

		if (tb[IFA_ADDRESS]) {
			struct nl_addr *a;

			a = nl_addr_alloc_attr(tb[IFA_ADDRESS], family);
			if (!a)
				goto errout_nomem;

			/* IPv6 sends the local address as IFA_ADDRESS with
			 * no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
			 * with IFA_ADDRESS being the peer address if they differ */
			if (!tb[IFA_LOCAL] || !nl_addr_cmp(a, addr->a_local)) {
				nl_addr_put(addr->a_local);
				addr->a_local = a;
				addr->ce_mask |= ADDR_ATTR_LOCAL;
			} else {
				addr->a_peer = a;
				addr->ce_mask |= ADDR_ATTR_PEER;
			}

			plen_addr = a;
		}
	}

	if (plen_addr)
		nl_addr_set_prefixlen(plen_addr, addr->a_prefixlen);

	/* IPv4 only */
	if (tb[IFA_BROADCAST]) {
		addr->a_bcast = nl_addr_alloc_attr(tb[IFA_BROADCAST], family);
		if (!addr->a_bcast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_BROADCAST;
	}

	/* IPv6 only */
	if (tb[IFA_MULTICAST]) {
		addr->a_multicast = nl_addr_alloc_attr(tb[IFA_MULTICAST],
						       family);
		if (!addr->a_multicast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_MULTICAST;
	}

	/* IPv6 only */
	if (tb[IFA_ANYCAST]) {
		addr->a_anycast = nl_addr_alloc_attr(tb[IFA_ANYCAST],
						       family);
		if (!addr->a_anycast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_ANYCAST;
	}

	if ((link_cache = __nl_cache_mngt_require("route/link"))) {
		struct rtnl_link *link;

		if ((link = rtnl_link_get(link_cache, addr->a_ifindex))) {
			rtnl_addr_set_link(addr, link);

			/* rtnl_addr_set_link incs refcnt */
			rtnl_link_put(link);
		}
	}

	err = pp->pp_cb((struct nl_object *) addr, pp);
errout:
	rtnl_addr_put(addr);

	return err;

errout_nomem:
	err = -NLE_NOMEM;
	goto errout;
#endif
}

static int mdb_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return nl_rtgen_request(sk, RTM_GETMDB, AF_BRIDGE, NLM_F_DUMP);
}

static void mdb_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
#if 0
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	char buf[128];

	link_cache = nl_cache_mngt_require_safe("route/link");

	if (addr->ce_mask & ADDR_ATTR_LOCAL)
		nl_dump_line(p, "%s",
			nl_addr2str(addr->a_local, buf, sizeof(buf)));
	else
		nl_dump_line(p, "none");

	if (addr->ce_mask & ADDR_ATTR_PEER)
		nl_dump(p, " peer %s",
			nl_addr2str(addr->a_peer, buf, sizeof(buf)));

	nl_dump(p, " %s ", nl_af2str(addr->a_family, buf, sizeof(buf)));

	if (link_cache)
		nl_dump(p, "dev %s ",
			rtnl_link_i2name(link_cache, addr->a_ifindex,
					 buf, sizeof(buf)));
	else
		nl_dump(p, "dev %d ", addr->a_ifindex);

	nl_dump(p, "scope %s",
		rtnl_scope2str(addr->a_scope, buf, sizeof(buf)));

	rtnl_addr_flags2str(addr->a_flags, buf, sizeof(buf));
	if (buf[0])
		nl_dump(p, " <%s>", buf);

	nl_dump(p, "\n");

	if (link_cache)
		nl_cache_put(link_cache);
#endif
}

static void mdb_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
#if 0
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	char buf[128];

	addr_dump_line(obj, p);

	if (addr->ce_mask & (ADDR_ATTR_LABEL | ADDR_ATTR_BROADCAST |
			     ADDR_ATTR_MULTICAST)) {
		nl_dump_line(p, "  ");

		if (addr->ce_mask & ADDR_ATTR_LABEL)
			nl_dump(p, " label %s", addr->a_label);

		if (addr->ce_mask & ADDR_ATTR_BROADCAST)
			nl_dump(p, " broadcast %s",
				nl_addr2str(addr->a_bcast, buf, sizeof(buf)));

		if (addr->ce_mask & ADDR_ATTR_MULTICAST)
			nl_dump(p, " multicast %s",
				nl_addr2str(addr->a_multicast, buf,
					      sizeof(buf)));

		if (addr->ce_mask & ADDR_ATTR_ANYCAST)
			nl_dump(p, " anycast %s",
				nl_addr2str(addr->a_anycast, buf,
					      sizeof(buf)));

		nl_dump(p, "\n");
	}

	if (addr->ce_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		nl_dump_line(p, "   valid-lifetime %s",
			     ci->aci_valid == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_valid * 1000,
					   buf, sizeof(buf)));

		nl_dump(p, " preferred-lifetime %s\n",
			ci->aci_prefered == 0xFFFFFFFFU ? "forever" :
			nl_msec2str(ci->aci_prefered * 1000,
				      buf, sizeof(buf)));

		nl_dump_line(p, "   created boot-time+%s ",
			     nl_msec2str(addr->a_cacheinfo.aci_cstamp * 10,
					   buf, sizeof(buf)));
		    
		nl_dump(p, "last-updated boot-time+%s\n",
			nl_msec2str(addr->a_cacheinfo.aci_tstamp * 10,
				      buf, sizeof(buf)));
	}
#endif
}

static uint64_t mdb_compare(struct nl_object *_a, struct nl_object *_b,
			     uint64_t attrs, int flags)
{
#if 0 
	struct rtnl_addr *a = (struct rtnl_addr *) _a;
	struct rtnl_addr *b = (struct rtnl_addr *) _b;
	uint64_t diff = 0;

#define ADDR_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, ADDR_ATTR_##ATTR, a, b, EXPR)

	diff |= ADDR_DIFF(IFINDEX,	a->a_ifindex != b->a_ifindex);
	diff |= ADDR_DIFF(FAMILY,	a->a_family != b->a_family);
	diff |= ADDR_DIFF(SCOPE,	a->a_scope != b->a_scope);
	diff |= ADDR_DIFF(LABEL,	strcmp(a->a_label, b->a_label));
	if (attrs & ADDR_ATTR_PEER) {
		if (   (flags & ID_COMPARISON)
		    && a->a_family == AF_INET
		    && b->a_family == AF_INET
		    && a->a_peer
		    && b->a_peer
		    && a->a_prefixlen == b->a_prefixlen) {
			/* when comparing two IPv4 addresses for id-equality, the network part
			 * of the PEER address shall be compared.
			 */
			diff |= ADDR_DIFF(PEER, nl_addr_cmp_prefix(a->a_peer, b->a_peer));
		} else
			diff |= ADDR_DIFF(PEER, nl_addr_cmp(a->a_peer, b->a_peer));
	}
	diff |= ADDR_DIFF(LOCAL,	nl_addr_cmp(a->a_local, b->a_local));
	diff |= ADDR_DIFF(MULTICAST,	nl_addr_cmp(a->a_multicast,
						    b->a_multicast));
	diff |= ADDR_DIFF(BROADCAST,	nl_addr_cmp(a->a_bcast, b->a_bcast));
	diff |= ADDR_DIFF(ANYCAST,	nl_addr_cmp(a->a_anycast, b->a_anycast));
	diff |= ADDR_DIFF(CACHEINFO,    memcmp(&a->a_cacheinfo, &b->a_cacheinfo,
	                                       sizeof (a->a_cacheinfo)));

	if (flags & LOOSE_COMPARISON)
		diff |= ADDR_DIFF(FLAGS,
				  (a->a_flags ^ b->a_flags) & b->a_flag_mask);
	else
		diff |= ADDR_DIFF(FLAGS, a->a_flags != b->a_flags);

#undef ADDR_DIFF

	return diff;
#endif
}

struct rtnl_mdb *rtnl_mdb_alloc(void)
{
	return (struct rtnl_mdb *) nl_object_alloc(&mdb_obj_ops);
}

void rtnl_mdb_put(struct rtnl_mdb *mdb)
{
	nl_object_put((struct nl_object *) mdb);
}

/** @} */

/**
 * @name Cache Management
 * @{
 */

int rtnl_mdb_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&rtnl_mdb_ops, sk, result);
}

# if 0
/**
 * Search for mdb entry in cache
 */
struct rtnl_mdb *rtnl_mdb_get(struct nl_cache *cache, int ifindex,
				struct nl_mdb *mdb)
{
	struct rtnl_mdb *entry;

	if (cache->c_ops != &rtnl_mdb_ops)
		return NULL;

	return NULL;
}
#endif

/** @} */

static int build_addr_msg()
{
  // fill message
	struct nl_msg *msg;
	return 0;
}

/**
 * @name Addition
 * @{
 * Build netlink request message to request addition of new address
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting the addition of a new
 * address. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - local address (rtnl_addr_set_local())
 *
 * The scope will default to universe except for loopback addresses in
 * which case a host scope is used if not specified otherwise.
 *
 * @note Free the memory after usage using nlmsg_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_mdb_build_add_request()
{
	return build_addr_msg(/*addr, RTM_NEWADDR, NLM_F_CREATE | flags, result*/);
}

/**
 * Request addition of new address
 * @arg sk		Netlink socket.
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_add_request()
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_mdb_add()
{
  return 0;
}

/** @} */

/**
 * @name Deletion
 * @{
 * Build a netlink request message to request deletion of an address
 * @arg addr		Address object to be deleteted.
 * @arg flags		Additional netlink message flags.
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting a deletion of an address.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - address family (rtnl_addr_set_family())
 *
 * Optional attributes:
 *   - local address (rtnl_addr_set_local())
 *   - label (rtnl_addr_set_label(), IPv4/DECnet only)
 *   - peer address (rtnl_addr_set_peer(), IPv4 only)
 *
 * @note Free the memory after usage using nlmsg_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_mdb_build_delete_request(/*struct rtnl_addr *addr, int flags,*/
					 /*struct nl_msg **result*/)
{
	/*return build_addr_msg(addr, RTM_DELADDR, flags, result);*/
  return 0;
}

/**
 * Request deletion of an address
 * @arg sk		Netlink socket.
 * @arg addr		Address object to be deleted.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_delete_request();
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_mdb_delete(/*struct nl_sock *sk, struct rtnl_addr *addr, int flags*/)
{
	return 0;
}

/** @} */

/**
 * @name Attributes
 * @{
 */

int rtnl_mdb_set_attribute(/*struct rtnl_addr *addr, const char *label*/)
{
	return 0;
}

/** @} */

static struct nl_object_ops mdb_obj_ops = {
	.oo_name		= "route/mdb",
	.oo_size		= sizeof(struct br_mdb_entry),
};

static struct nl_af_group mdb_groups[] = {
	{ AF_BRIDGE,	RTNLGRP_MDB },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_mdb_ops = {
	.co_name		= "route/mdb",
	.co_hdrsize		= sizeof(struct br_mdb_entry),
	.co_msgtypes		= {
					{ RTM_NEWMDB, NL_ACT_NEW, "new"},
					{ RTM_DELMDB, NL_ACT_DEL, "del"},
					{ RTM_GETMDB, NL_ACT_GET, "get"},
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_obj_ops		= &mdb_obj_ops,
};

static void __init mdb_init(void)
{
	nl_cache_mngt_register(&rtnl_mdb_ops);
}

static void __exit mdb_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_mdb_ops);
}

/** @} */
