/*
 * lib/route/link/bonding.c	Bonding Link Module
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2011-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup bonding Bonding
 *
 * @details
 * \b Link Type Name: "bond"
 *
 * @route_doc{link_bonding, Bonding Documentation}
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/route/link/bonding.h>
#include <netlink-private/route/link/api.h>

/** @cond SKIP */
#define BONDING_ATTR_MODE		(1 << 0)
#define BONDING_ATTR_ACTIVE_SLAVE	(1 << 1)
#define BONDING_ATTR_PRIMARY		(1 << 2)
#define BONDING_ATTR_XMIT_HASH_POLICY	(1 << 3)

struct bonding_info
{
	uint8_t  bi_mode;
	uint32_t bi_active_slave;
	uint32_t bi_primary;
	uint8_t  bi_xmit_hash_policy;
	uint32_t ce_mask;
};

/** @endcond */

static struct nla_policy bonding_nl_policy[IFLA_BOND_MAX+1] = {
	[IFLA_BOND_MODE]             = { .type = NLA_U8 },
	[IFLA_BOND_ACTIVE_SLAVE]     = { .type = NLA_U32 },
	[IFLA_BOND_PRIMARY]          = { .type = NLA_U32 },
	[IFLA_BOND_XMIT_HASH_POLICY] = { .type = NLA_U8 },
};

static int bonding_alloc(struct rtnl_link *link)
{
	struct bonding_info *info;

	if (link->l_info)
		memset(link->l_info, 0, sizeof(*info));
	else {
		if ((info = calloc(1, sizeof(*info))) == NULL)
			return -NLE_NOMEM;

		link->l_info = info;
	}
	return 0;
}

static int bonding_parse(struct rtnl_link *link, struct nlattr *data,
			struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_BOND_MAX+1];
	struct bonding_info *info;
	int err;

	NL_DBG(3, "Parsing bonding info\n");

	if ((err = nla_parse_nested(tb, IFLA_BOND_MAX, data, bonding_nl_policy)) < 0)
		goto errout;

	if ((err = bonding_alloc(link)) < 0)
		goto errout;

	info = link->l_info;

	if (tb[IFLA_BOND_MODE]) {
		info->bi_mode = nla_get_u8(tb[IFLA_BOND_MODE]);
		info->ce_mask |= BONDING_ATTR_MODE;
	}

	if (tb[IFLA_BOND_ACTIVE_SLAVE]) {
		info->bi_active_slave = nla_get_u32(tb[IFLA_BOND_ACTIVE_SLAVE]);
		info->ce_mask |= BONDING_ATTR_ACTIVE_SLAVE;
	}

	if (tb[IFLA_BOND_PRIMARY]) {
		info->bi_primary = nla_get_u32(tb[IFLA_BOND_PRIMARY]);
		info->ce_mask |= BONDING_ATTR_PRIMARY;
	}

	if (tb[IFLA_BOND_XMIT_HASH_POLICY]) {
		info->bi_xmit_hash_policy = nla_get_u32(tb[IFLA_BOND_XMIT_HASH_POLICY]);
		info->ce_mask |= BONDING_ATTR_XMIT_HASH_POLICY;
	}

	err = 0;
errout:
	return err;
}

static void bonding_free(struct rtnl_link *link)
{
	free(link->l_info);
	link->l_info = NULL;
}

static int bonding_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct boding_info *vdst, *vsrc = src->l_info;
	int err;

	dst->l_info = NULL;
	if ((err = rtnl_link_set_type(dst, "bond")) < 0)
		return err;
	vdst = dst->l_info;

	if (!vdst || !vsrc)
		return -NLE_NOMEM;

	memcpy(vdst, vsrc, sizeof(struct bonding_info));

	return 0;
}

static int bonding_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct bonding_info *info = link->l_info;
	struct nlattr *data;

	if (!(data = nla_nest_start(msg, IFLA_INFO_DATA)))
		return -NLE_MSGSIZE;

	if (info->ce_mask & BONDING_ATTR_MODE)
		NLA_PUT_U8(msg, IFLA_BOND_MODE, info->bi_mode);
	if (info->ce_mask & BONDING_ATTR_ACTIVE_SLAVE)
		NLA_PUT_U32(msg, IFLA_BOND_ACTIVE_SLAVE, info->bi_active_slave);
	if (info->ce_mask & BONDING_ATTR_PRIMARY)
		NLA_PUT_U32(msg, IFLA_BOND_PRIMARY, info->bi_primary);
	if (info->ce_mask & BONDING_ATTR_XMIT_HASH_POLICY)
		NLA_PUT_U8(msg, IFLA_BOND_XMIT_HASH_POLICY, info->bi_xmit_hash_policy);

	nla_nest_end(msg, data);

nla_put_failure:
	return 0;
}

/**
 * Allocate link object of type bond
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_bond_alloc(void)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if ((err = rtnl_link_set_type(link, "bond")) < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Create a new kernel bonding device
 * @arg sock		netlink socket
 * @arg name		name of bonding device or NULL
 * @arg opts		bonding options (currently unused)
 *
 * Creates a new bonding device in the kernel. If no name is
 * provided, the kernel will automatically pick a name of the
 * form "type%d" (e.g. bond0, vlan1, etc.)
 *
 * The \a opts argument is currently unused. In the future, it
 * may be used to carry additional bonding options to be set
 * when creating the bonding device.
 *
 * @note When letting the kernel assign a name, it will become
 *       difficult to retrieve the interface afterwards because
 *       you have to guess the name the kernel has chosen. It is
 *       therefore not recommended to not provide a device name.
 *
 * @see rtnl_link_bond_enslave()
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_bond_add(struct nl_sock *sock, const char *name,
		       struct rtnl_link *opts)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_bond_alloc()))
		return -NLE_NOMEM;

	if (!name && opts)
		name = rtnl_link_get_name(opts);

	if (name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sock, link, NLM_F_CREATE);

	rtnl_link_put(link);

	return err;
}

/**
 * Add a link to a bond (enslave)
 * @arg sock		netlink socket
 * @arg master		ifindex of bonding master
 * @arg slave		ifindex of slave link to add to bond
 *
 * This function is identical to rtnl_link_bond_enslave() except that
 * it takes interface indices instead of rtnl_link objcets.
 *
 * @see rtnl_link_bond_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_enslave_ifindex(struct nl_sock *sock, int master,
				   int slave)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_bond_alloc()))
		return -NLE_NOMEM;

	rtnl_link_set_ifindex(link, slave);
	rtnl_link_set_master(link, master);
	
	if ((err = rtnl_link_change(sock, link, link, 0)) < 0)
		goto errout;

	rtnl_link_put(link);

	/*
	 * Due to the kernel not signaling whether this opertion is
	 * supported or not, we will retrieve the attribute to see  if the
	 * request was successful. If the master assigned remains unchanged
	 * we will return NLE_OPNOTSUPP to allow performing backwards
	 * compatibility of some sort.
	 */
	if ((err = rtnl_link_get_kernel(sock, slave, NULL, &link)) < 0)
		return err;

	if (rtnl_link_get_master(link) != master)
		err = -NLE_OPNOTSUPP;

errout:
	rtnl_link_put(link);

	return err;
}

/**
 * Add a link to a bond (enslave)
 * @arg sock		netlink socket
 * @arg master		bonding master
 * @arg slave		slave link to add to bond
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message adding the slave to
 * the master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_bond_enslave_ifindex()
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_enslave(struct nl_sock *sock, struct rtnl_link *master,
			   struct rtnl_link *slave)
{
	return rtnl_link_bond_enslave_ifindex(sock,
				rtnl_link_get_ifindex(master),
				rtnl_link_get_ifindex(slave));
}

/**
 * Release a link from a bond
 * @arg sock		netlink socket
 * @arg slave		slave link to be released
 *
 * This function is identical to rtnl_link_bond_release() except that
 * it takes an interface index instead of a rtnl_link object.
 *
 * @see rtnl_link_bond_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_release_ifindex(struct nl_sock *sock, int slave)
{
	return rtnl_link_bond_enslave_ifindex(sock, 0, slave);
}

/**
 * Release a link from a bond
 * @arg sock		netlink socket
 * @arg slave		slave link to be released
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message releasing the slave from
 * its master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_bond_release_ifindex()
 * @see rtnl_link_bond_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_release(struct nl_sock *sock, struct rtnl_link *slave)
{
	return rtnl_link_bond_release_ifindex(sock,
				rtnl_link_get_ifindex(slave));
}

static struct rtnl_link_info_ops bonding_info_ops = {
	.io_name		= "bond",
	.io_alloc		= bonding_alloc,
	.io_parse		= bonding_parse,
	.io_clone		= bonding_clone,
	.io_put_attrs		= bonding_put_attrs,
	.io_free		= bonding_free,
};

/** @cond SKIP */
#define IS_BONDING_LINK_ASSERT(link) \
	if ((link)->l_info_ops != &bonding_info_ops) { \
		APPBUG("Link is not a bonding link. set type \"bond\" first."); \
		return -NLE_OPNOTSUPP; \
	}
/** @endcond */

/**
 * Set bonding mode
 * @arg link		Link object
 * @arg mode		bond mode
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_set_mode(struct rtnl_link *link, uint8_t mode)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	info->bi_mode = mode;
	info->ce_mask |= BONDING_ATTR_MODE;

	return 0;
}

/**
 * Get bonding mode
 * @arg link		Link object
 * @arg mode		bond mode
 *
 * @return bond mode, 0 if not set or a negative error code.
 */
int rtnl_link_bond_get_mode(struct rtnl_link *link, uint8_t *mode)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	if (!(info->ce_mask & BONDING_ATTR_MODE))
		return -NLE_NOATTR;

	if (mode)
		*mode = info->bi_mode;

	return 0;
}

/**
 * Set bonding active slave
 * @arg link		Link object
 * @arg active_slave	active slave
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_set_active_slave(struct rtnl_link *link, uint32_t active_slave)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	info->bi_active_slave = active_slave;
	info->ce_mask |= BONDING_ATTR_ACTIVE_SLAVE;

	return 0;
}

/**
 * Get bonding mode
 * @arg link		Link object
 *
 * @return bond mode, 0 if not set or a negative error code.
 */
int rtnl_link_bond_get_active_slave(struct rtnl_link *link, uint32_t *active_slave)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	if (!(info->ce_mask & BONDING_ATTR_ACTIVE_SLAVE))
		return -NLE_NOATTR;

	if (active_slave)
		*active_slave = info->bi_active_slave;

	return 0;
}

/**
 * Set bonding primary
 * @arg link		Link object
 * @arg primary		primary
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_set_primary(struct rtnl_link *link, uint32_t primary)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	info->bi_primary = primary;
	info->ce_mask |= BONDING_ATTR_PRIMARY;

	return 0;
}

/**
 * Get bonding primary
 * @arg link		Link object
 *
 * @return primary, 0 if not set or a negative error code.
 */
int rtnl_link_bond_get_primary(struct rtnl_link *link, uint32_t *primary)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	if (!(info->ce_mask & BONDING_ATTR_PRIMARY))
		return -NLE_NOATTR;

	if (primary)
		*primary = info->bi_primary;

	return 0;
}

/**
 * Set bonding mode
 * @arg link		Link object
 * @arg mode		bond mode
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_bond_set_xmit_hash_policy(struct rtnl_link *link, uint8_t policy)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	info->bi_xmit_hash_policy = policy;
	info->ce_mask |= BONDING_ATTR_XMIT_HASH_POLICY;

	return 0;
}

/**
 * Get bonding mode
 * @arg link		Link object
 * @arg mode		bond mode
 *
 * @return bond mode, 0 if not set or a negative error code.
 */
int rtnl_link_bond_get_xmit_hash_policy(struct rtnl_link *link, uint8_t *policy)
{
	struct bonding_info *info = link->l_info;

	IS_BONDING_LINK_ASSERT(link);

	if (!(info->ce_mask & BONDING_ATTR_XMIT_HASH_POLICY))
		return -NLE_NOATTR;

	if (policy)
		*policy = info->bi_xmit_hash_policy;

	return 0;
}

static void __init bonding_init(void)
{
	rtnl_link_register_info(&bonding_info_ops);
}

static void __exit bonding_exit(void)
{
	rtnl_link_unregister_info(&bonding_info_ops);
}

/** @} */
