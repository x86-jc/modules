/************************************************
 * Title:		ku								*
 * Creator:		John J. Coleman					*
 * Creation Date:	2026.04.20					*
 * Modified Date:	2026.04.20					*
 * 												*
 * Description:									*
 * Read from and write to char device.			*
 *												*
 ************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_AUTHOR("John J. Coleman");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read from and write to character device.");

static int major;
static char text[64];

/* returns amount of bytes read successfully */
static ssize_t ku_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	int not, delta, copied = (length + *offset) < sizeof(text) ? length : sizeof(text) - *offset;

	pr_info("ku: reading %ld bytes, copied %d.  offset is %lld.\n", length, copied, *offset);
	
	not = copy_to_user(buffer, &text[*offset], copied);
	delta = copied - not;
	if(not)
	{
		pr_warn("ku: copied %d bytes.\n", delta);
	}

	*offset = delta;

	return delta;
}

/* buffer is constant because we only read it. */
static ssize_t ku_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
	int not, delta, copied = (length + *offset) < sizeof(text) ? length : sizeof(text) - *offset;

	pr_info("ku: writing %ld bytes, copied %d.  offset is %lld.\n", length, copied, *offset);
	
	not = copy_from_user(&text[*offset], buffer, copied);
	delta = copied - not;
	if(not)
	{
		pr_warn("ku: copied %d bytes.\n", delta);
	}

	return delta;
}
static struct file_operations fops = 
{
	.read = ku_read;
	.write = ku_write;
};

static int __init ku_init(void)
{
	major = register_chrdev(0, "ku", &fops);
	if(major < 0)
	{
		pr_err("ku: error registering char device.\n");
		return major;
	}
	printk("ku: major device number: %d\n", major);
	return 0;
}

static void __exit ku_exit(void)
{
	unregister_chrdev(major, "ku");
}

module_init(ku_init);
module_exit(ku_exit);
