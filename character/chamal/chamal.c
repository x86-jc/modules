/************************************************
 * Title:		chamal		        			*
 * Creator:		John J. Coleman					*
 * Creation Date:	2026.04.27					*
 * Modified Date:	2026.04.27					*
 * 												*
 * Description:									*
 * Memory allocation within character device 	*
 * module.										*
 ************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>

MODULE_AUTHOR("John J. Coleman");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example of memory allocation within a module.");

#define MEMSIZE 64

static dev_t device_number;
static struct cdev chamal_cdev;
static struct class *chamal_class;

static int chamal_open(struct inode *inode, struct file *filp)
{
	/* open files allow us to pass private data. */
	/* (number of bytes, flag for allocation) */
	filp->private_data = kmalloc(MEMSIZE, GFP_KERNEL);
	if(!filp->private_data)
	{
		pr_err("chamal: out of memory.\n");
		return -ENOMEM;
	}

	return 0;
}

static int chamal_release(struct inode *inode, struct file *filp)
{
	kfree(filp->private_data);
	return 0;
}

static ssize_t chamal_read(struct file *filp, char __user *user, size_t length, loff_t *offset)
{
	char *text = filp->private_data;
	int not, delta, copied = (length + *offset) < MEMSIZE ? length : (MEMSIZE - *offset);

	pr_info("chamal: read called, read %ld bytes, copied %d bytes, offset is %lld.\n", length, copied, *offset);
	if(*offset >= sizeof(text))
	{
		return 0;
	}

	not = copy_to_user(user, &text[*offset], copied);
	delta = copied - not;
	if(not)
	{
		pr_warn("chamal_cdev: only copied %d bytes\n", delta);
	}

	*offset += delta;

	return delta;
}

static ssize_t chamal_write(struct file *filp, const char __user *user, size_t length, loff_t *offset)
{
	char *text = filp->private_data;
	int not, delta, copied = (length + *offset) < MEMSIZE ? length : (MEMSIZE - *offset);

	pr_info("chamal: write called, write %ld bytes, copied %d bytes, offset is %lld.\n", length, copied, *offset);
	if(*offset >= sizeof(text))
	{
		return 0;
	}

	not = copy_from_user(&text[*offset], user, copied);
	delta = copied - not;
	if(not)
	{
		pr_warn("chamal: copied %d bytes.\n", delta);
	}

	*offset += delta;
	return delta;
}

static struct file_operations fops = 
{
	.read = chamal_read,
	.write = chamal_write,
	.open = chamal_open,
	.release = chamal_release,
	.llseek = default_llseek		// default_llseek resets offset based on how lseek is called in user program
};

static int __init chamal_init(void)
{
	int status;
#ifdef STATIC_DEVICE_NUMBER
	device_number = STATIC_DEVICE_NUMBER;
	status = register_chrdev_region(device_number, MINORMASK + 1, "chamal");
#else
	status = alloc_chrdev_region(&device_number, 0, MINORMASK + 1, "chamal");
#endif

	if(status)
	{
		pr_err("chamal: error reserving region of device numbers.\n");
		return status;
	}

	cdev_init(&chamal_cdev, &fops);
	chamal_cdev.owner = THIS_MODULE;

	status = cdev_add(&chamal_cdev, device_number, MINORMASK + 1);
	if(status)
	{
		pr_err("chamal: failed to add cdev.\n");
		goto free_device_number;
	}

	pr_info("chamal: registered character device with major number %d starting with minor number %d.\n", MAJOR(device_number), MINOR(device_number));

	chamal_class = class_create("chamal_class");
	if(!chamal_class)
	{
		pr_err("chamal: failed to create chamal class.\n");
		status = ENOMEM;
		goto delete_cdev;
	}

	if(!device_create(chamal_class, NULL, device_number, NULL, "chamal%d", 0);)
	{
		pr_err("chamal: faled to create device chamal0.\n");
		status = ENOMEM;
		goto delete_class;
	}

	pr_info("chamal: created device under /sys/class/chamal_class/chamal0.\n");

	return 0;

delete_class:
	class_unregister(chamal_class);
	class_destroy(chamal_class);

delete_cdev:
	cdev_del(&chamal_cdev);

free_device_number:
	unregister_chrdev_region(device_number, MINORMASK + 1);
	return status;
}

static void __exit chamal_exit(void)
{
	device_destroy(chamal_class, device_number);
	class_unregister(chamal_class);
	class_destroy(chamal_class);
	cdev_del(&chamal_cdev);
	unregister_chrdev_region(device_number, MINORMASK + 1);
}

module_init(chamal_init);
module_exit(chamal_exit);
