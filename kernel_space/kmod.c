#include <linux/socket.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h> 
#include <net/genetlink.h>

#include "../genltest.h"

#define MSG_MAX_LEN 1024
static struct genl_family genl_fam;

// Handle GENLTEST_CMD_ECHO messages received
static int echo_doit(struct sk_buff *skb, struct genl_info *info) {
    int             ret = 0;
    void           *hdr;
    struct sk_buff *msg;

    // Check if attribute present and print it
    if (info->attrs[GENLTEST_A_MSG]) {
        char *str = nla_data(info->attrs[GENLTEST_A_MSG]);
        pr_info("MESSAGE RECEIVED: %s\n", str);
    } else {
        pr_info("EMPTY MESSAGE RECEIVED\n");
    }

    // Allocate a new buffer for the reply
    msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!msg) {
        pr_err("FAILED TO ALLOCATE MESSAGE BUFFER\n");
        return -ENOMEM;
    }

    // Put the Generic Netlink header
    hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &genl_fam, 0, GENLTEST_CMD_ECHO);
    if (!hdr) {
        pr_err("FAILED TO CREATE GENETLINK HEADER\n");
        nlmsg_free(hdr);
        return -EMSGSIZE;
    }

    // Put the Generic Netlink message
    if ((ret = nla_put_string(msg, GENLTEST_A_MSG, "Kernelly hii"))) {
        pr_err("FAILED TO CREATE MESSAGE STRING\n");
        genlmsg_cancel(msg, hdr);
        nlmsg_free(msg);
        goto out;
    }

    // Finalize and send message
    genlmsg_end(msg, hdr);
    ret = genlmsg_reply(msg, info);
    pr_info("SENT MESSAGE SUCCESSFULLY\n");

out:
    return ret;
}

static int echo_ping(const char *buf, size_t cnt) {
    int             ret = 0;
    void           *hdr;
    struct sk_buff *skb = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);

    if (unlikely(!skb)) {
        pr_err("FAILED TO ALLOCATE MEMORY FOR GENL MESSAGE\n");
        return -ENOMEM;
    }

    // Put the Generic Netlink header
    hdr = genlmsg_put(skb, 0, 0, &genl_fam, 0, GENLTEST_CMD_ECHO);
    if (unlikely(!hdr)) {
        pr_err("FAILED TO ALLOCATE MEMORY FOR THE GENL HEADER\n");
        nlmsg_free(skb);
        return -ENOMEM;
    }

    // Put the Generic Netlink message
    if ((ret = nla_put_string(skb, GENLTEST_A_MSG, buf))) {
        pr_err("UNABLE TO CREATE MESSAGE STRING\n");
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return ret;
    }

    // Finalize the message
    genlmsg_end(skb, hdr);

    // Send message via multicast to 0th grp in array
    ret = genlmsg_multicast(&genl_fam, skb, 0, 0, GFP_KERNEL);
    if (ret == -ESRCH) {
        pr_warn("MULTICAST MESSAGE SENT BUT NO ONE LISTENED\n");
    } else if (ret) {
        pr_err("FAILED TO SEND MULTICAST GENL MESSAGE\n");
    } else {
        pr_info("MULTICAST MESSAGE SENT\n");
    }

    return ret;
}

static ssize_t ping_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t cnt) {
    int max = cnt > MSG_MAX_LEN ? MSG_MAX_LEN : cnt;
    echo_ping(buf, max);
    return max;
}

// Attribute validation policy for echo command
static struct nla_policy echo_pol[GENLTEST_A_MAX + 1] = {
    [GENLTEST_A_MSG] = { .type = NLA_NUL_STRING },
};

// Generic Netlink family operations
static struct genl_ops genl_ops[] = {
    {
        .cmd    = GENLTEST_CMD_ECHO,
        .policy = echo_pol,
        .doit   = echo_doit,
    },
};

// Family multicast groups
static const struct genl_multicast_group genl_mcgrps[] = {
    { .name = GENLTEST_MC_GRP_NAME },
};

// Generic Netlink family
static struct genl_family genl_fam = {
    .name     = GENLTEST_GENL_NAME,
    .version  = GENLTEST_GENL_VERSION,
    .maxattr  = GENLTEST_A_MAX,
    .ops      = genl_ops,
    .n_ops    = ARRAY_SIZE(genl_ops),
    .mcgrps   = genl_mcgrps,
    .n_mcgrps = ARRAY_SIZE(genl_mcgrps),
};

static struct kobject *kobj;
static struct kobj_attribute ping_attr = __ATTR_WO(ping);

static int __init start(void) {
    int ret = 0;
    pr_info("Init start\n");

    kobj = kobject_create_and_add("genltest", kobj);
    if (unlikely(!kobj)) {
        pr_err("UNABLE TO CREATE KERNEL OBJECT\n");
        return -ENOMEM;
    }

    ret = sysfs_create_file(kobj, &ping_attr.attr);
    if (unlikely(ret)) {
        pr_err("UNABLE TO CREATE SYSFS FILE\n");
        kobject_put(kobj);
        return ret;
    }

    ret = genl_register_family(&genl_fam);
    if (unlikely(ret)) {
        pr_crit("FAILED TO REGISTER GENERIC NETLINK FAMILY\n");
        sysfs_remove_file(kobj, &ping_attr.attr);
        kobject_put(kobj);
    }

    pr_info("Info end\n");
    return ret;
    // int ret = genl_register_family(&genl_fam);
    // if (unlikely(ret)) {
    //     pr_crit("FAILED TO REGISTER GENERIC NETLINK FAMILY\n");
    // }

    // int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    // return 0;
}

static void __exit stop(void) {
    if (unlikely(genl_unregister_family(&genl_fam))) {
        pr_err("FAILED TO UNREGISTER GENERIC NETLINK FAMILY\n");
    }

    sysfs_remove_file(kobj, &ping_attr.attr);
    kobject_put(kobj);

    pr_info("Exiting\n");
}

module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
