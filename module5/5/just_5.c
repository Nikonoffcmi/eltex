#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#define NETLINK_USER 31

static struct sock *nl_sk = NULL;

static void hello_nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    const char *msg = "Who're you?";
    int res;

    printk(KERN_INFO "Netlink: Received message\n");


    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid;
    
    printk(KERN_INFO "Kernel: Received payload: %s\n", (char *)nlmsg_data(nlh));

    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    memcpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_ERR "Error sending message to user\n");
}

struct netlink_kernel_cfg cfg = {
    .groups = 1,
    .input = hello_nl_recv_msg,
};

static int __init hello_init(void)
{
    printk(KERN_INFO "Initializing Netlink module\n");
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -ENOMEM;
    }

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Exiting Netlink module\n");
    if (nl_sk)
        netlink_kernel_release(nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chumadevsky N");