#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


#define MEM_SIZE 1024

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
uint8_t *kernel_buffer;

static int etx_open(struct inode *inode, struct file *fi);
static int etx_release(struct inode *inode, struct file *fi);
static ssize_t etx_read(struct file *fi, char __user *buff, size_t len, loff_t *off);
static ssize_t etx_write(struct file *fi, const char __user *buff, size_t len, loff_t *off);
static int __init hello_world_init(void);
static void __exit hello_world_exit(void);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open,
    .release = etx_release,
    .read = etx_read,
    .write = etx_write,
};

static int etx_open(struct inode *inode, struct file *fi)
{
    if ((kernel_buffer = kmalloc(MEM_SIZE, GFP_KERNEL)) == 0)
    {
        printk(KERN_ERR "Can not allocate memory in kernelQ\n");
        return -1;
    }
    printk(KERN_INFO "kernel Module open!\n");
    return 0;
}

static int etx_release(struct inode *inode, struct file *fi)
{
    kfree(kernel_buffer);
    printk(KERN_INFO "Kernel Module release!\n");
    return 0;
}

static ssize_t etx_read(struct file *fi, char __user *buff, size_t len, loff_t *off)
{
    copy_to_user(buff, kernel_buffer, MEM_SIZE);
    printk(KERN_INFO "kernel Module read!\n");
    return 0;
}

static ssize_t etx_write(struct file *fi, const char __user *buff, size_t len, loff_t *off)
{
    copy_from_user(kernel_buffer, buff, len);
    printk(KERN_INFO "kernel Module write!\n");
    return len;
}

static int __init hello_world_init(void)
{
    if (alloc_chrdev_region(&dev, 0, 1, "etx_Dev1") < 0)
    {
        printk(KERN_ERR "Can not allocate major number!\n");
        return -1;
    }
    printk(KERN_INFO "Major: %d Minor: %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&etx_cdev, &fops);
    if (IS_ERR(cdev_add(&etx_cdev, dev, 1)))
    {
        printk(KERN_ERR "Can not add device into kernel!\n");
        goto r_class;
    }

    if (IS_ERR(dev_class = class_create("etx_class1")))
    {
        printk(KERN_ERR "Can not allocate class!\n");
        goto r_class;
    }

    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device1")))
    {
        printk(KERN_ERR "Can not allocate device!\n");
        goto r_device;
    }
    printk(KERN_INFO "Kernel Module inserted successfully!\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit hello_world_exit(void)
{
    kfree(kernel_buffer);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Kernel Module exited successfully!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hoangquocbao");
MODULE_DESCRIPTION("Kernel Module simple!");
MODULE_VERSION("1.2");