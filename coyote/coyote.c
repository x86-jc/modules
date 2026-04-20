/************************************************
 * Title:			coyote						*
 * Creator:			John Coleman				*
 * Creation Date:		2025.01.05				*
 * Modified Date:		2025.01.21				*
 * 												*
 * Description:									*
 * Basic character device driver.				*
 *												*
 ************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Coleman");
MODULE_DESCRIPTION("Basic character device driver.");
MODULE_VERSION("1.0.0");

dev_t dev;
unsigned int result;
unsigned int coyote_major;
unsigned int coyote_minor;
unsigned int dev_count = 1;

static int coyote_open(struct inode *inode, struct file *filp);
static int coyote_close(struct inode *inode, struct file *filp);
static ssize_t coyote_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
static ssize_t coyote_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset);

static struct coyote_dev
{
	struct coyote_cset *data;
	int crate;
	int cset;
	unsigned long size;
	unsigned int access_key;
	struct semaphore sem;
	struct cdev cdev;
};

static struct file_operations coyote_fops =
{
	.owner		= THIS_MODULE,
	.open		= coyote_open,
	.release	= coyote_close,
	.read		= coyote_read,
	.write		= coyote_write
};

static int coyote_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "coyote: open called.\n");

	struct coyote_dev *dev;
	
	/* container_of returns a pointer to the containing structure */
	/* container_of(pointer, container_type, container_field) */
	/* below, we return a pointer to coyote_dev which contains cdev */
	dev = container_of(inode->i_cdev, struct coyote_dev, cdev);
	filp->private_data = dev; 			/* for other methods */

	/* trim to 0 the length of device if open was write-only */
	if((filp->f_flags & O_ACCMODE)==O_WRONLY)
	{
		coyote_trim(dev);
	}

	return 0;
}

static int coyote_close(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "coyote: close called.\n");
	return 0;
}

static ssize_t coyote_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	// read data from device and copy it to user space
	printk(KERN_INFO "coyote: read called.\n");
}

static ssize_t coyote_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	// write data from user space to device
	printk(KERN_INFO "coyote: write called.\n");
}

static void coyote_setup_cdev(struct coyote_dev *dev, int index)
{
	int err, devno = MKDEV(coyote_major, coyote_minor+index);
	cdev_init(&dev->cdev, &coyote_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &coyote_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	if(err)
	{
		printk(KERN_WARNING "Error %d adding coyote %d", err, index);
	}
}

static int __init coyote_init(void)
{
//	int coyote_major = register_chrdev(MAJOR_NUMBER, "chardev", &coyote_fops);
	result = alloc_chrdev_region(&dev, coyote_minor, dev_count, "chardev");
	coyote_major = MAJOR(dev);
	if(coyote_major<0)
	{
		printk(KERN_ERR "Failed to register character device.\n");
		return coyote_major;
	}
  
	printk(KERN_INFO "coyote: character device registered with major number %d\n", coyote_major);
	printk(KERN_INFO "coyote: character device registered with minor number %d\n", coyote_minor);
	printk(KERN_INFO "coyote: calling cdev setup.\n");

	return 0;
}

static void __exit coyote_exit(void)
{
	//unregister_chrdev(MAJOR_NUMBER, "chardev");
	unregister_chrdev_region(dev, dev_count);
	printk(KERN_INFO "coyote: character device unregistered.\n");
}

module_init(coyote_init);
module_exit(coyote_exit);
