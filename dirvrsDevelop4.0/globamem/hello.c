#include<linux/module.h>
#include<linux/types.h>
#include<linux/fs.h>
#include<linux/erron.h>
#include<linux/mm.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<asm/io.h>
#include<asm/system.h>
#include<asm/uaccess.h>
#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x1
#define  GLOBALMEM_MAJOR 254

static globalmem_major=GLOBALMEM_MAJOR;
struct globalmem_dev
{
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];
}

struct globalmem_dev *globalmem_devp;
int globalmem_open(struct inode *inode,struct file *filp)
{
	filp->private_data=globalmem_devp;
	return 0;
}
int globalmem_release(struct inode *inode,struct file *filp)
{
	return 0;
}
static int globalmem_ioctl(struct inode *inode,struct file *filp,unsigned int cmd,unsigned long arg)
{
	struct globalmem_dev *dev=filp->private_data;
	switch(cmd)
	{
		case MEM_CLEAR:
			memset(dev->mem,0,GLOBALMEM_SIZE);
			printk(KERN_INFO"globalmem is set to zero\n");
		break;
		default:
			return -EINVAL;
	}
	return 0;
}
static ssize_t globalmem_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos)
{
	unsigned long p=*ppos;
	unsigned int count=size;
	int ret=0;
	struct globalmem_dev *dev=filp->private_data;

}
static int __init hello_init(void)
{
	printk(KERN_INFO"Hello world enter\n");
	return 0;
}
module_init(hello_init);
static void __exit hello_exit(void)
{
	printk(KERN_INFO"Hello world exit\n");

}
module_exit(hello_exit);
MODULE_AUTHOR("chao");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple hello world module");
MODULE_ALIAS("a simple module");


