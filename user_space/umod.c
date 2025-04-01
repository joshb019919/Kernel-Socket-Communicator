#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>

#include "../genltest.h"

#define prerr(...) fprintf(stderr, "error: " __VA_ARGS__)
static int echo_reply_handler(struct nl_msg *, void *);

// Allocate Netlink socket and connect to Generic Netlink
static int conn(struct nl_sock **sk) {
    *sk = nl_socket_alloc();
    if (!sk) {
        return -NLE_NOMEM;
    }

    return genl_connect(*sk);
}

static inline int set_cb(struct nl_sock *sk) {
    return nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, echo_reply_handler, NULL);
}

static int echo_reply_handler(struct nl_msg *msg, void *arg) {
    int                err     = 0;
    struct genlmsghdr *genlhdr = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr     *tb[GENLTEST_A_MAX + 1];

    // Parse attributes
    err = nla_parse(tb, GENLTEST_A_MAX, genlmsg_attrdata(genlhdr, 0), genlmsg_attrlen(genlhdr, 0), NULL);
    if (err) {
        prerr("UNABLE TO PARSE MESSAGE: %s\n", strerror(-err));
        return NL_SKIP;
    }

    // Make sure there's a payload
    if (!tb[GENLTEST_A_MSG]) {
        prerr("MESSAGE ATTRIBUTE MISSING FROM MESSAGE\n");
        return NL_SKIP;
    }

    // Print message
    printf("Message Received: %s\n", nla_get_string(tb[GENLTEST_A_MSG]));
    return NL_OK;
}

// Send unicast GENL_CMD_ECHO message request
static int send_echo_msg(struct nl_sock *sk, int fam)
{
	int	           err = 0;
	struct nl_msg *msg = nlmsg_alloc();

	if (!msg) {
		return -NLE_NOMEM;
	}

	// Put GENL header inside message buffer
	void *hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, fam, 0, 0, GENLTEST_CMD_ECHO, GENLTEST_GENL_VERSION);
	if (!hdr) {
		return -NLE_MSGSIZE;
	}

	// Put string inside message
	err = nla_put_string(msg, GENLTEST_A_MSG, "Hello from User Space, Netlink!");
	if (err < 0) {
		return -err;
	}

	printf("Message Sent\n");

	// Send message
	err = nl_send_auto(sk, msg);
	err = err >= 0 ? 0 : err;
	nlmsg_free(msg);

	return err;
}

// Disconnect and release socket
static void disconn(struct nl_sock *sk)
{
	nl_close(sk);
	nl_socket_free(sk);
}

int main(void) {
	int	ret = 1;
	struct nl_sock *ucsk, *mcsk;
    /*
    * We use one socket to receive asynchronous "notifications" over
    * multicast group, and another for ops. We do this so that we don't mix
    * up responses from ops with notifications to make handling easier.
    */
    if ((ret = conn(&ucsk)) || (ret = conn(&mcsk))) {
        prerr("FAILED TO CONNECT TO GENERIC NETLINK\n");
        goto out;
    }

    int fam = genl_ctrl_resolve(ucsk, GENLTEST_GENL_NAME);
    if (fam < 0) {
        prerr("FAILED TO RESOLVE GENERIC NETLINK FAMILY: %s\n", strerror(-fam));
        goto out;
    }

    nl_socket_disable_seq_check(mcsk);

    // Resolve the multicast group
    int mcgrp = genl_ctrl_resolve_grp(mcsk, GENLTEST_GENL_NAME, GENLTEST_MC_GRP_NAME);
    if (mcgrp < 0) {
        prerr("FAILED TO RESOLVE GENERIC MULTICAST GRUOP: %s\n", strerror(-mcgrp));
        goto out;
    }

    // Join the multicast group
    if ((ret = nl_socket_add_membership(mcsk, mcgrp) < 0)) {
        prerr("FAILED TO JOIN MULTICAST GRUOP: %s\n", strerror(-ret));
        goto out;
    }

    if ((ret = set_cb(ucsk)) || (ret = set_cb(mcsk))) {
        prerr("FAILED TO SET CALLBACK: %s\n", strerror(-ret));
        goto out;
    }

    // Send unicast message and wait for response
    if ((ret = send_echo_msg(ucsk, fam))) {
        prerr("FAILED TO SEND MESSAGE: %s\n", strerror(-ret));
    }

    printf("Listening for messages...\n");
    nl_recvmsgs_default(ucsk);

    // Listen for notifications
    while (1) {
        nl_recvmsgs_default(mcsk);
    }

    ret = 0;

out:
    disconn(ucsk);
    disconn(mcsk);
    return ret;
}
