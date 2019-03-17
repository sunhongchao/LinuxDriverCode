#include<linux/module.h>
#include<linux/fs.h>
#include<linux/mm.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/workqueue.h>
#include<linux/uaccess.h>

#define SECOND_MAJOR 248 /*预设的 second 的主设备号*/

static int second_major = SECOND_MAJOR;

 /*second 设备结构体*/
 struct second_dev
 {
 	struct cdev cdev; /*cdev 结构体*/
 	atomic_t counter;/* 一共经历了多少秒？ */
    struct delayed_work delayed_worker;
 };

 struct second_dev *second_devp; /*设备结构体指针*/

 /*定时器处理函数*/
 static void delayed_work_handler(struct work_struct *work)
 {
 	atomic_inc(&second_devp->counter);
 	printk(KERN_INFO " jiffies is %ld on work\n", jiffies);
	schedule_delayed_work(&second_devp->delayed_worker,5*HZ);
 }

 /*文件打开函数*/
 int second_open(struct inode *inode, struct file *filp)
 {
 /*初始化定时器*/
 	INIT_DELAYED_WORK(&second_devp->delayed_worker,delayed_work_handler);
	printk(KERN_INFO " jiffies is %ld when open\n", jiffies);
 	schedule_delayed_work(&second_devp->delayed_worker,5*HZ);
 	return 0;
 }
 /*文件释放函数*/
 int second_release(struct inode *inode, struct file *filp)
 {
 	cancel_delayed_work(&second_devp->delayed_worker);
	
 	return 0;
 }
 	/*globalfifo 读函数*/
static ssize_t second_read(struct file *filp,char __user *buf, size_t count,loff_t *ppos)
{
 	int counter;
 	counter = atomic_read(&second_devp->counter);
	if(put_user(counter, (int*)buf))
		 return -EFAULT;
	else
		 return sizeof(unsigned int);
	
}
	
	 /*文件操作结构体*/
static const struct file_operations second_fops =
{
	.owner = THIS_MODULE,
	.open = second_open,
	.release = second_release,
	.read = second_read,
};
	
	 /*初始化并注册 cdev*/
static void second_setup_cdev(struct second_dev *dev, int index)
{
	int err, devno = MKDEV(second_major, index);
		
	cdev_init(&dev->cdev, &second_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &second_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}
	
	 /*设备驱动模块加载函数*/
static int __init second_init(void)
{
	 int ret=0;
	 dev_t devno = MKDEV(second_major, 0);
	
	 /* 申请设备号*/
	 if (second_major)
	 	ret = register_chrdev_region(devno, 1, "second");
	 if(ret<0) /* 动态申请设备号 */
	 {
	 	ret = alloc_chrdev_region(&devno, 0, 1, "second");
	 	second_major = MAJOR(devno);
	 }
	 if (ret < 0)
		return ret;
	 /* 动态申请设备结构体的内存*/
	second_devp = kmalloc(sizeof(struct second_dev), GFP_KERNEL);
 	if (!second_devp) /*申请失败*/
	 {
		 ret = - ENOMEM;
	 	goto fail_malloc;
	 }
	
	 memset(second_devp, 0, sizeof(struct second_dev));
	
	 second_setup_cdev(second_devp, 0);
	
	 return 0;
	
fail_malloc: unregister_chrdev_region(devno, 1);
	return ret;
}

	
	 /*模块卸载函数*/
static void __exit second_exit(void)
 {
	 cdev_del(&second_devp->cdev); /*注销 cdev*/
	 kfree(second_devp); /*释放设备结构体内存*/
	 unregister_chrdev_region(MKDEV(second_major, 0), 1); /*释放设备号*/
}
module_init(second_init);
module_exit(second_exit);	
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Song Baohua");
	

	

	






















