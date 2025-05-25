#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MAX_BUF_SIZE 10

static int len;
static int temp;
static char *msg;

static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp)
{
    if (count > temp)
        count = temp;

    temp -= count;
    
    if (copy_to_user(buf, msg, count)) {
        pr_err("Failed to copy data to user\n");
        return -EFAULT;
    }

    if (count == 0)
        temp = len;

    return count;
}

static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    if (count > MAX_BUF_SIZE) {
        pr_err("Write size exceeds buffer limit\n");
        return -EINVAL;
    }

    if (copy_from_user(msg, buf, count)) {
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }

    len = count;
    temp = len;
    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = read_proc,
    .proc_write = write_proc,
};

static void create_new_proc_entry(void)
{
    proc_create("proc", 0, NULL, &proc_fops);
    msg = kmalloc(MAX_BUF_SIZE, GFP_KERNEL);
}

static int __init proc_init(void)
{
    create_new_proc_entry();
    return 0;
}

static void __exit proc_cleanup(void)
{
    remove_proc_entry("proc", NULL);
    kfree(msg);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chumadevsky N");
module_init(proc_init);
module_exit(proc_cleanup);