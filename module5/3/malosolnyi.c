#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h> /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>  /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h> /* For vc_cons */
#include <linux/vt_kern.h>
#include <linux/timer.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>

struct timer_list my_timer;
struct tty_driver *my_driver;
static int _kbledstatus = 0;

static struct kobject *kbleds_kobj;
static unsigned int led_mask = 0;

#define BLINK_DELAY HZ / 5
#define RESTORE_LEDS 0x00

static ssize_t mask_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    return sprintf(buf, "%d\n", led_mask);
}

static ssize_t mask_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    unsigned int val;
    int ret = kstrtouint(buf, 0, &val);
    
    if (ret < 0)
        return ret;
    
    led_mask = val & 0x07;
    return count;
}

static struct kobj_attribute mask_attribute = __ATTR(mask, 0660, mask_show,
                                                    mask_store);

static void my_timer_func(struct timer_list *ptr)
{
    int *pstatus = &_kbledstatus;
    
    if (*pstatus == led_mask)
        *pstatus = RESTORE_LEDS;
    else
        *pstatus = led_mask;

    if (my_driver->ops->ioctl)
        my_driver->ops->ioctl(vc_cons[fg_console].d->port.tty, 
                            KDSETLED, *pstatus);
    
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
}

static int __init kbleds_init(void)
{
    int error = 0;

    printk(KERN_INFO "kbleds: Initializing module\n");

    kbleds_kobj = kobject_create_and_add("systest", kernel_kobj);
    if (!kbleds_kobj)
        return -ENOMEM;

    error = sysfs_create_file(kbleds_kobj, &mask_attribute.attr);
    if (error)
    {
        kobject_put(kbleds_kobj);
        return error;
    }

    my_driver = vc_cons[fg_console].d->port.tty->driver;

    timer_setup(&my_timer, my_timer_func, 0);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);

    return error;
}

static void __exit kbleds_exit(void)
{
    printk(KERN_INFO "kbleds: Unloading module\n");
    del_timer(&my_timer);

    if (my_driver->ops->ioctl)
        my_driver->ops->ioctl(vc_cons[fg_console].d->port.tty,
                            KDSETLED, RESTORE_LEDS);
    
    kobject_put(kbleds_kobj);
}

MODULE_LICENSE("GPL");
module_init(kbleds_init);
module_exit(kbleds_exit);