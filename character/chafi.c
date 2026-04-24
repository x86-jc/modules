/************************************************
 * Title:		chafi		        			*
 * Creator:		John J. Coleman					*
 * Creation Date:	2026.04.24					*
 * Modified Date:	2026.04.24					*
 * 												*
 * Description:									*
 * Create a character device and create device	*
 * file in module.								*
 ************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_AUTHOR("John J. Coleman");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Creating device file through module.");

static dev_t device_number;
static struct cdev chafi_cdev;
static struct class *chafi_class;

static ssize_t chafi_read(struct file *filp, char __user *user, size_t length, loff_t *offset)
{
	pr_info("chafi: read called.\n")
	return 0;
};

static struct file_operations fops = 
{
	.read = chafi_read;
}

static int __init chafi_init(void)
{
	int status;

#ifdef STATIC_DEVICE_NUNBER
	device_number = STATIC_DEVICE_NUMBER;
	status = register_chrdev_region(device_number, MINORMASK + 1, "chafi");
#else
	status = alloc_chrdev_region(&device_number, 0, MINORMASK + 1, "chafi");
#endif
	if(status)
	{
		pr_err("chafi: error reserving region of device numbers.\n");
		return status;
	}

	cdev_init(&chafi_cdev, &fops);
	chafi_cdev.owner = THIS_MODULE;

	status = cdev_add(&chafi_cdev, device_number, MINORMASK + 1);
	if(status)
	{
		pr_err("chafi: error adding cdev.\n");
		goto free_device_number;
	}

	pr_info("chafi: registered character device for major %d starting with minor %d\n", MAJOR(device_number), MINOR(device_number));

	chafi_class = class_create("chafi_class");
	if(!chafi_class)
	{
		pr_err("chafi: could not create chafi class.\n");
		status = ENOMEM;
		goto delete_cdev;
	}

	if(!device_create(chafi_class, NULL, device_number, NULL, "chafi%d", 0))
	{
		pr_err("chafi: failed to create device chafi0.\n");
		status = ENOMEM;
		goto delete_class;
	}
	
	pr_info("chafi: created device under /sys/class/chafi_class/chafi0.\n");

	return 0;

delete_class:
	class_unregister(chafi_class);
	class_destroy(chafi_class);

delete_cdev:
	cdev_del(&chafi_cdev);

free_device_number:
	unregister_chrdev_region(device_number, MINORMASK + 1);
	return status;
}

static void __exit chafi_exit(void)
{
	device_destroy(chafi_class, device_number);
	class_unregister(chafi_class);
	class_destroy(chafi_class);
	cdev_del(&chafi_cdev);
	unregister_chrdev_region(device_number, MINORMASK + 1);
}

module_init(chafi_init);
module_exit(chafi_exit);
