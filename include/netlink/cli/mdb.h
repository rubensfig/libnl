/*
 * netlink/cli/addr.h    CLI Address Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_MDB_H_
#define __NETLINK_CLI_MDB_H_

#include <netlink/route/mdb.h>

#define nl_cli_mdb_alloc_cache(sk) \
		nl_cli_alloc_cache_flags((sk), "mdb", NL_CACHE_AF_ITER, rtnl_mdb_alloc_cache)

//extern struct rtnl_mdb *nl_cli_mdb_alloc(void);

//extern void nl_cli_mdb_parse_family(struct rtnl_mdb *, char *);

#endif
