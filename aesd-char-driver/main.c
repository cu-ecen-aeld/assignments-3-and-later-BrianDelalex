/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("BrianDelalex"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    struct aesd_dev *dev = &aesd_device;

    filp->private_data = dev;
    /**
     * TODO: handle open
     */
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    struct aesd_dev *dev = (struct aesd_dev *)filp->private_data;
    size_t entry_offset = 0;
    char *kbuf;

    down_read(&dev->rw_sem);
    
    kbuf = kmalloc(count, GFP_KERNEL);
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

    while (retval < count) {
        struct aesd_buffer_entry *entry = 
            aesd_circular_buffer_find_entry_offset_for_fpos(
                &dev->circular_buffer,
                *f_pos + retval,
                &entry_offset
            );
        if (!entry) {
            if (retval == 0) {
                kfree(kbuf);
                up_read(&dev->rw_sem);
                return 0;
            }
            break;
        }
        while (entry_offset < entry->size && retval < count) {
            kbuf[retval] = entry->buffptr[entry_offset];
            entry_offset++;
            retval++;
        }
    }
    up_read(&dev->rw_sem);
    unsigned long byte_copied = copy_to_user(buf, kbuf, retval);
    kfree(kbuf);
    if (byte_copied != 0) {
        PDEBUG("%s(l.%d): %ld bytes has not been copied.\n", __FUNCTION__, __LINE__, byte_copied);
        return retval - byte_copied;
    }
    *f_pos = *f_pos + retval;

    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    bool cmd_complete = 0;
    ssize_t retval = -ENOMEM;
    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;
    size_t pending_count = 0;
    char *to_write = NULL;

    down_write(&dev->rw_sem);
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    if (dev->pending_write) {
        for (; dev->pending_write[pending_count]; pending_count++);
        to_write = kmalloc(count + pending_count + 1, GFP_KERNEL);
        if (!to_write) {
            up_write(&dev->rw_sem);
            return retval;
        }
        to_write[count + pending_count] = 0;
        memcpy(to_write, dev->pending_write, pending_count);
        kfree(dev->pending_write);
        dev->pending_write = NULL;
    }

    char *kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) {
        if (to_write)
            kfree(to_write);
        up_write(&dev->rw_sem);
        return retval;
    }
    memset(kbuf, 0, count + 1);

    if (copy_from_user(kbuf, buf, count) != 0) {
        PDEBUG("%s(.%d), copy_from_user error\n", __FUNCTION__, __LINE__);
    }

    for (int i = 0; i < count; i++) {
        if (kbuf[i] == '\n') {
            cmd_complete = 1;
            break;
        }
    }

    if (!to_write) {
        to_write = kbuf;
    } else {
        memcpy(&(to_write[pending_count]), kbuf, count);
        kfree(kbuf);
    }

    if (cmd_complete) {
        struct aesd_buffer_entry entry;
        entry.buffptr = to_write;
        entry.size = count + pending_count;
        PDEBUG("adding %s to circular buffer\n", entry.buffptr);
        aesd_circular_buffer_add_entry(&dev->circular_buffer, &entry);
    } else {
        PDEBUG("adding %s to pending write\n", to_write);
        dev->pending_write = to_write;
    }
    up_write(&dev->rw_sem);
    return count;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    PDEBUG("init function called.\n");
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    aesd_circular_buffer_init(&aesd_device.circular_buffer);
    init_rwsem(&aesd_device.rw_sem);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    PDEBUG("cleanup function called.\n");
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    for (int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
        if (aesd_device.circular_buffer.entry[i].buffptr) {
            kfree(aesd_device.circular_buffer.entry[i].buffptr);
        }
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
