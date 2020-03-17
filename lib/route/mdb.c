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

static void mdb_free_data(struct nl_object *obj)
{
  // Cleans the functions
}

static void mdb_constructor(struct nl_object *obj)
{
  struct rtnl_mdb *_mdb = (struct rtnl_mdb *)obj;

  nl_init_list_head(&_mdb->mdb_entry_list);
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
  return 0;
}

static struct nla_policy mdb_policy[MDBA_MAX+1] = {
	[MDBA_MDB]	= { .type = NLA_NESTED },
};

static struct nla_policy mdb_db_policy[MDBA_MDB_MAX+1] = {
	[MDBA_MDB_ENTRY]	= { .type = NLA_NESTED },
};

static struct nla_policy mdb_entry_policy[MDBA_MDB_ENTRY_MAX+1] = {
	[MDBA_MDB_ENTRY_INFO]	= { .type = NLA_UNSPEC },
};

static int mdb_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *nlh, struct nl_parser_param *pp)
{

	int err = 0;

  struct rtnl_mdb* _mdb = rtnl_mdb_alloc();
	if (!_mdb)
		return -NLE_NOMEM;

  struct nlattr *tb[MDBA_MAX+1];

	err = nlmsg_parse(nlh, sizeof(struct br_port_msg), tb, MDBA_MAX, mdb_policy); /*struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[], int maxtype, const struct nla_policy *policy*/

  _mdb->ce_msgtype = nlh->nlmsg_type;
  struct br_port_msg *_port;
  _port = nlmsg_data(nlh);

  _mdb->ifindex = _port->ifindex;
  _mdb->family = _port->family;

  if(tb[MDBA_MDB]) {
    struct nlattr *db_attr[MDBA_MDB_MAX+1];

    nla_parse_nested(db_attr, MDBA_MDB_MAX, tb[MDBA_MDB], mdb_db_policy); /* struct nlattr *tb[], int maxtype, struct nlattr *head, int len, const struct nla_policy *policy*/

    if(db_attr[MDBA_MDB_ENTRY]) {
      struct nlattr *entry_attr[MDBA_MDB_ENTRY_MAX+1];
      struct rtnl_mdb_entry *_nentry = rtnl_mdb_entry_alloc();
      
      nla_parse_nested(entry_attr, MDBA_MDB_ENTRY_MAX, db_attr[MDBA_MDB_ENTRY], mdb_entry_policy);

	    struct br_mdb_entry* entry;
      entry = nla_data(entry_attr[MDBA_MDB_ENTRY_INFO]);

      _nentry->ifindex = entry->ifindex;
      _nentry->vid = entry->vid;
      _nentry->state = entry->state;
      _nentry->proto = ntohs(entry->addr.proto);

      // debugging
      struct nl_addr * _addr = nl_addr_alloc(255);
      nl_addr_parse("239.0.1.13", AF_INET, &_addr);
      _nentry->addr = _addr;

      rtnl_mdb_add_entry(_mdb, _nentry);
    }
  }

  err = pp->pp_cb((struct nl_object *) _mdb, pp);

  return 0;
}

static int mdb_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return nl_rtgen_request(sk, RTM_GETMDB, AF_BRIDGE, NLM_F_DUMP);
}

static void mdb_entry_dump_line(struct rtnl_mdb_entry *entry, struct nl_dump_params *p) {
  printf("mdb entry dump line: ifindex %d\n", entry->ifindex);
  printf("mdb entry dump line: vid %d\n", entry->vid);
  printf("mdb entry dump line: proto 0x%04x\n", entry->proto);
}

static void mdb_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
  struct rtnl_mdb *mdb = (struct rtnl_mdb *) obj;

  printf("mdb dump line: ifindex : %d\n", mdb->ifindex);
  printf("mdb dump line: family  : %d\n", mdb->family);

  struct rtnl_mdb_entry* _mdb;
	nl_list_for_each_entry(_mdb, &mdb->mdb_entry_list, mdb_list) {
		p->dp_ivar = NH_DUMP_FROM_ONELINE;
		mdb_entry_dump_line(_mdb, p);
	}
}

static void mdb_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
  printf("mdb dump details");
}

static uint64_t mdb_compare(struct nl_object *_a, struct nl_object *_b,
			     uint64_t attrs, int flags)
{
  return 0;
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

/**
 * Build a neighbour cache including all neighbours currently configured in the kernel.
 * @arg sock		Netlink socket.
 * @arg result		Pointer to store resulting cache.
 * @arg flags		Flags to apply to cache before filling
 *
 * Allocates a new neighbour cache, initializes it properly and updates it
 * to include all neighbours currently configured in the kernel.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_mdb_alloc_cache_flags(struct nl_sock *sock, struct nl_cache **result,
				 unsigned int flags)
{
	struct nl_cache * cache;
	int err;

	cache = nl_cache_alloc(&rtnl_mdb_ops);
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

uint32_t rtnl_mdb_get_ifindex(struct rtnl_mdb *mdb)
{
	return mdb->ifindex;
}

uint8_t rtnl_mdb_get_family(struct rtnl_mdb *mdb)
{
	return mdb->family;
}

void rtnl_mdb_add_entry(struct rtnl_mdb *mdb, struct rtnl_mdb_entry *entry) {
  nl_list_add_tail(&entry->mdb_list ,&mdb->mdb_entry_list);
}

void rtnl_mdb_foreach_entry(struct rtnl_mdb *mdb,
                            void (*cb)(struct rtnl_mdb_entry *, void *),
                            void *arg ) {
  struct rtnl_mdb_entry * entry;
  nl_list_for_each_entry(entry, &mdb->mdb_entry_list, mdb_list) {
			cb(entry, arg);
		}
}

int rtnl_mdb_entry_get_ifindex(struct rtnl_mdb_entry *mdb_entry) {
  return mdb_entry->ifindex;
}

int rtnl_mdb_entry_get_vid(struct rtnl_mdb_entry *mdb_entry) {
  return mdb_entry->vid;
}

int rtnl_mdb_entry_get_state(struct rtnl_mdb_entry *mdb_entry) {
  return mdb_entry->state;
}

struct nl_addr * rtnl_mdb_entry_get_addr(struct rtnl_mdb_entry *mdb_entry) {
  return mdb_entry->addr;
}

struct nl_addr * rtnl_mdb_entry_get_proto(struct rtnl_mdb_entry *mdb_entry) {
  return mdb_entry->proto;
}
/** @} */

static struct nl_object_ops mdb_obj_ops = {
	.oo_name		= "route/mdb",
	.oo_size		= sizeof(struct rtnl_mdb), // FIX ME
  .oo_constructor = mdb_constructor,
	.oo_dump = {
	    [NL_DUMP_LINE] 	= mdb_dump_line,
	    [NL_DUMP_DETAILS]	= mdb_dump_details,
	},
};

struct rtnl_mdb *rtnl_mdb_alloc(void)
{
  return (struct rtnl_mdb *) nl_object_alloc(&mdb_obj_ops);
}

static struct nl_object_ops mdb_entry_obj_ops = {
	.oo_name		= "route/mdb_entry",
	.oo_size		= sizeof(struct rtnl_mdb_entry),
};

struct rtnl_mdb_entry *rtnl_mdb_entry_alloc(void)
{
  struct rtnl_mdb_entry *_entry;
  
  _entry = calloc(1, sizeof(*_entry));
  return _entry;  
}

static struct nl_af_group mdb_groups[] = {
	{ AF_BRIDGE,	RTNLGRP_MDB },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_mdb_ops = {
	.co_name		= "route/mdb",
	.co_hdrsize		= 10, // FIX ME
	.co_msgtypes		= {
					{ RTM_NEWMDB, NL_ACT_NEW, "new"},
					{ RTM_DELMDB, NL_ACT_DEL, "del"},
					{ RTM_GETMDB, NL_ACT_GET, "get"},
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
  .co_groups		= mdb_groups,
  .co_request_update	= mdb_request_update,
  .co_msg_parser = mdb_msg_parser,
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
