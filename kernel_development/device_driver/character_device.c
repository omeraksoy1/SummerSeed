#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/*  This code closely follows the sample code from a guide posted online by Robert W. Oliver II
	at https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234. I followed 
	said guide for educational purposes and all credit goes to the author himself. */

#define DEVICE_NAME "character_device"
#define EXAMPLE_MSG "Hello world\n"
#define MSG_BUFFER_LEN 15

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char * msg_ptr;

static struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
	int bytes_read = 0;
	
	if (*msg_ptr == 0) {
		msg_ptr = msg_buffer;
	}
	
	while (len && *msg_ptr) {
		put_user(*(msg_ptr++), buffer++);
		len--;
		bytes_read++;
	}
	return bytes_read;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
	printk(KERN_ALERT "Operation not supported, device is read-only.");
	return -EINVAL;
}

static int device_open(struct inode *inode, struct file *file) {
	/* Will return busy if already opened. */
	if (device_open_count) {
		return -EBUSY;
	}
	/* otherwise, open it */
	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	/* Close device, decrement count back to 0 */
	device_open_count++;
	module_put(THIS_MODULE);
	return 0;
}

static int __init character_device_init(void) {
	strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);
	msg_ptr = msg_buffer;
	major_num = register_chrdev(0, "character_device", &file_ops);
	
	if (major_num < 0) {
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num;
	} else {
		printk(KERN_INFO "character_device module loaded w/device major no. %d\n", major_num);
		return 0;
	}
}

static void __exit character_device_exit(void) {
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Bye world\n");
}

module_init(character_device_init);
module_exit(character_device_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SummerSeed internship assignment");
MODULE_AUTHOR("Omer Faruk Aksoy");


