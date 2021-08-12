#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE( "GPL");
MODULE_DESCRIPTION( "SummerSeed internship assignment");
MODULE_AUTHOR("Omer Faruk Aksoy");

static int __init my_module_init(void) {
	printk(KERN_INFO "Loading module...\n");
	return 0;
}

static void __exit my_module_exit(void) {
	printk(KERN_INFO "Removing module.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

