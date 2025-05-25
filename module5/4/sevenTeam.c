#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "chardev"
#define BUF_LEN 80   
#define SUCCESS 0

enum
{
    CDEV_NOT_USED = 0,
    CDEV_EXCLUSIVE_OPEN = 1,
};

static int major;
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);
static struct class *cls;

static char user_buffer[BUF_LEN];
static size_t user_buffer_len = 0;
static loff_t read_offset = 0;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);


static struct file_operations chardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

static int __init chardev_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

    if (major < 0)
    {
        pr_alert("Registering char device failed with %d\n", major);
        return major;
    }

    pr_info("I was assigned major number %d.\n", major);

    cls = class_create(DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);
    return SUCCESS;
}

static void __exit chardev_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Device unloaded\n");
}

static int device_open(struct inode *inode, struct file *file)
{
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;
    read_offset = 0;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
    atomic_set(&already_open, CDEV_NOT_USED);
    module_put(THIS_MODULE);
    return SUCCESS;
}

static ssize_t device_read(struct file *filp,  
                           char __user *buffer, 
                           size_t length,      
                           loff_t *offset) 
{
    ssize_t bytes_read = 0;
    if (read_offset >= user_buffer_len) {
        read_offset = 0;
        return SUCCESS;
    }

    bytes_read = min(user_buffer_len - read_offset, length);
    if (copy_to_user(buffer, user_buffer + read_offset, bytes_read)) {
        return -EFAULT;
    }

    read_offset += bytes_read;
    *offset = read_offset;

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    if (len > BUF_LEN) {
        pr_info("Write exceeds buffer size\n");
        return -EINVAL;
    }


    if (copy_from_user(user_buffer, buff, len)) {
        return -EFAULT;
    }

    user_buffer_len = len;
    read_offset = 0;

    return len;   
}

module_init(chardev_init);
module_exit(chardev_exit);
MODULE_LICENSE("GPL");