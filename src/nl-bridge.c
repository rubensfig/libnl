/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * src/nl-bridge.c     Bridge utility
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/bridge_vlan.h>

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_link *bridge;
	struct rtnl_bridge_vlan *bvlan = nl_cli_bridge_vlan_alloc();
	struct nl_cache *bvlan_cache;
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};
	int err;

	sock = nl_cli_alloc_socket();
	nl_cli_socket_disable_auto_ack(sock);
	nl_cli_connect(sock, NETLINK_ROUTE);
	bvlan_cache = nl_cli_bridge_vlan_alloc_cache_flags(sock, 0);

	nl_cli_bridge_vlan_parse_ifindex(bvlan, 4);
	nl_cache_dump(bvlan_cache, &dp);

	printf("end\n");

	return 0;
}
