/************************************************
 * Title:		chariot		         *
 * Creator:		John J. Coleman		*
 * Creation Date:	2026.04.22		*
 * Modified Date:	2026.04.22		*
 * 						*
 * Description:					*
 * Create a character device.			*
 *						*
 ************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_AUTHOR("John J. Coleman");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Manual creation of a character device.");

static dev_t dev_nr;
static struct cdev chariot_cdev;

static ssize_t chariot_read(struct file *filp, char __user *user, size_t length, loff_t *offset)
{
	pr_info("chariot: read invoked.\n");

	return 0;
}

static struct file_operations fops = 
{
	.read = chariot_read
};

static int __init chariot_init(void)
{
	int status;
#ifdef STATIC_DEVNR
	dev_nr = STATIC_DEVNR;
	/* int (dev_t from, unsigned count, const char *name) */
	status = register_chrdev_region(dev_nr, MINORMASK + 1, "chariot");
#else
	/* int (dev_t *dev, unsigned baseminor, unsigned count, const char *name) */
	status = alloc_chrdev_region(&dev_nr, 0, MINORMASK + 1, "chariot");
#endif
	if(status)
	{
		pr_err("chariot: error reserving region of device numbers.\n");
		return status;
	}
	
	cdev_init(&chariot_cdev, &fops);
	chariot_cdev.owner = THIS_MODULE;

	status = cdev_add(&chariot_cdev, dev_nr, MINORMASK + 1);
	if(status)
	{
		pr_err("chariot: error adding cdev.\n");
		goto free_dev_nr;
	}

	pr_info("chariot: registered character device with major: %d minor starting at: %d\n", MAJOR(dev_nr), MINOR(dev_nr));

free_dev_nr:
	unregister_chrdev_region(dev_nr, MINORMASK + 1);
	return status;
}

static void __exit chariot_exit(void)
{
	cdev_del(&chariot_cdev);
	unregister_chrdev_region(dev_nr, MINORMASK + 1);
}

module_init(chariot_init);
module_exit(chariot_exit);
