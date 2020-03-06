#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/cli/utils.h>
#include <signal.h>

#include <netlink-private/cache-api.h>

#include <linux/netlink.h>

static int quit = 0;

static struct nl_dump_params dp = {
	.dp_type = NL_DUMP_LINE,
};


static void change_cb(struct nl_cache *cache, struct nl_object *obj,
		      int action, void *data)
{
	if (action == NL_ACT_NEW)
		printf("NEW ");
	else if (action == NL_ACT_DEL)
		printf("DEL ");
	else if (action == NL_ACT_CHANGE)
		printf("CHANGE ");

	nl_object_dump(obj, &dp);
}

static void sigint(int arg)
{
	quit = 1;
}

int main(int argc, char *argv[])
{
	struct nl_cache_mngr *mngr;
	struct nl_cache *cache;
	int err;

	dp.dp_fd = stdout;

	signal(SIGINT, sigint);

	err = nl_cache_mngr_alloc(NULL, NETLINK_ROUTE, NL_AUTO_PROVIDE, &mngr);
	if (err < 0)
		nl_cli_fatal(err, "Unable to allocate cache manager: %s",
			     nl_geterror(err));

  err = nl_cache_mngr_add(mngr, argv[1], &change_cb, NULL, &cache);
  if (err < 0)
    nl_cli_fatal(err, "Unable to add cache %s: %s",
           argv[0], nl_geterror(err));
#if 0
	for (i = 1; i < argc; i++) {
		err = nl_cache_mngr_add(mngr, argv[i], &change_cb, NULL, &cache);
		if (err < 0)
			nl_cli_fatal(err, "Unable to add cache %s: %s",
				     argv[i], nl_geterror(err));
	}
#endif

	return 0;
}
