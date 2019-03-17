#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#define GLOBALFIFO_SIZE 0x1000 /*全局内存最大4KB*/
#define MEM_CLEAR 0x1  /*清零全局内存*/
#define GLOBALMEM_MAJOR 251    /*预设的globalmem的主设备号*/

static int globalmem_major = GLOBALMEM_MAJOR;
 /*globalmem设备结构体*/
struct globalfifo_dev
{
    struct cdev cdev; /*cdev结构体*/
	unsigned int current_len;//fifo length
    unsigned char mem[GLOBALFIFO_SIZE]; /*全局内存*/
	struct semaphore sem;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
};

struct globalfifo_dev *globalfifo_devp; /*设备结构体指针*/
  /*文件打开函数*/
int globalmem_open(struct inode *inode, struct file *filp)
 {
	/*将设备结构体指针赋值给文件私有数据指针*/
    filp->private_data = globalfifo_devp;
   return 0; }
 	/*文件释放函数*/
int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* ioctl设备控制函数*/
 static int globalmem_ioctl(struct inode *inodep, struct file *filp, 					unsigned int cmd, unsigned long arg)
 {
   struct globalfifo_dev *dev = filp->private_data;/*获得设备结构体指针*/

   switch (cmd)
   {
     case MEM_CLEAR:
	if(down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}
    memset(dev->mem, 0, GLOBALFIFO_SIZE);
	up(&dev->sem);
    printk(KERN_INFO "globalmem is set to zero\n");
    break;

     default:
      return  - EINVAL;
   }
    return 0;
 }

 /*读函数*/
static ssize_t globalmem_read(struct file *filp, char __user *buf,
				size_t size,loff_t *ppos)
 {
    
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data; /*获得设备结构体指针*/
	DECLARE_WAITQUEUE(wait,current);
	down(&dev->sem);
	add_wait_queue(&dev->r_wait,&wait);
   
	if(dev->current_len==0)
	{
		if(filp->f_flags&O_NONBLOCK)
		{
			ret=-EAGAIN;
			goto out;
		}
	
		__set_current_state(TASK_INTERRUPTIBLE);
		up(&dev->sem);
		schedule();
		if(signal_pending(current))
		{
			ret=-ERESTARTSYS;
			goto out2;
		}
		down(&dev->sem);
	}
	if(count>dev->current_len)
		count=dev->current_len;
	
    /*内核空间→用户空间*/
    if (copy_to_user(buf, dev->mem, count))
    {
		ret =  -EFAULT;
		goto out;
    }
   else
   {
    	memcpy(dev->mem,dev->mem+count,dev->current_len-count);
		dev->current_len-=count;
		printk(KERN_INFO"read %d bytes(s),current_len:%d\n",count,dev->current_len);
		wake_up_interruptible(&dev->w_wait);
		ret=count;
   }
	out:up(&dev->sem);
	out2:remove_wait_queue(&dev->w_wait,&wait);
	set_current_state(TASK_RUNNING);
    return ret;
 }

  /*写函数*/
static ssize_t globalmem_write(struct file *filp, const char __user 								*buf,size_t size, loff_t *ppos)
{
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data; /*获得设备结构体指针*/
	DECLARE_WAITQUEUE(wait,current);
	down(&dev->sem);
	add_wait_queue(&dev->w_wait,&wait);
    if(dev->current_len==GLOBALFIFO_SIZE)
	{
		if(filp->f_flags & O_NONBLOCK)
		{
			ret=-EAGAIN;
			goto out;
		}
	
		__set_current_state(TASK_INTERRUPTIBLE);
		up(&dev->sem);
		schedule();
		if(signal_pending(current))
		{
			ret=-ERESTARTSYS;
			goto out2;
		}
		down(&dev->sem);
	}
    if(count>GLOBALFIFO_SIZE-dev->current_len)
		count=GLOBALFIFO_SIZE-dev->current_len;

   if (copy_from_user(dev->mem + dev->current_len, buf, count))
	{	
     	ret =  - EFAULT;
		goto out;
	}
   else
   {
   		dev->current_len+=count;
		printk(KERN_INFO"written %d bytes(s),current_len:%d\n",count,dev->current_len);
		wake_up_interruptible(&dev->r_wait);
		ret=count;
   }
	out:up(&dev->sem);
	out2:remove_wait_queue(&dev->w_wait,&wait);
	set_current_state(TASK_RUNNING);
    return ret;
 }

 /* seek文件定位函数*/
 static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig) {
   loff_t ret = 0;

   switch (orig)
{
     case 0:   /*相对文件开始位置偏移*/
       if (offset < 0)
      {
        ret =  - EINVAL;
        break;
       }
       if ((unsigned int)offset > GLOBALFIFO_SIZE)
       {
         ret =  - EINVAL;
        break;
      }
       filp->f_pos = (unsigned int)offset;
      ret = filp->f_pos;
    case 1:   /*相对文件当前位置偏移*/
       if ((filp->f_pos + offset) > GLOBALFIFO_SIZE)
       {
         ret =  - EINVAL;
        break;
       }
      if ((filp->f_pos + offset) < 0)
       {
        ret =  - EINVAL;
         break;
       }
       filp->f_pos += offset;
       ret = filp->f_pos;
      break;
     default:
       ret =  - EINVAL;
      break;
   }
   return ret;
}

 /*文件操作结构体*/
 static  struct file_operations globalmem_fops ={
   .owner = THIS_MODULE,
   .llseek = globalmem_llseek,
   .read = globalmem_read,
   .write = globalmem_write,
   .ioctl = globalmem_ioctl,
   .open = globalmem_open,
   .release = globalmem_release,
 };

 /*初始化并注册cdev*/
 static void globalmem_setup_cdev(struct globalfifo_dev *dev, int index)
 {
   int err, devno = MKDEV(globalmem_major, index);

   cdev_init(&dev->cdev, &globalmem_fops);
   dev->cdev.owner = THIS_MODULE;

   dev->cdev.ops = &globalmem_fops;
   err = cdev_add(&dev->cdev, devno, 1);
   if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
 }

 /*设备驱动模块加载函数*/
static int __init  globalmem_init(void)
 {
  int result=0;
  dev_t devno = MKDEV(globalmem_major, 0);

   /*申请设备号*/
  if (globalmem_major)
     	result = register_chrdev_region(devno, 1, "globalfifo");
   if(result<0) /*动态申请设备号*/
   {
    	result = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
    	globalmem_major = MAJOR(devno);
   }
   if (result < 0)
    return result;

   /*动态申请设备结构体的内存*/
   globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
   if (!globalfifo_devp)    /*申请失败*/
  {
    result =  - ENOMEM;
     goto fail_malloc;
   }
   memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

   globalmem_setup_cdev(globalfifo_devp, 0);
	init_MUTEX(&globalfifo_devp->sem);
	init_waitqueue_head(&globalfifo_devp->r_wait);
	init_waitqueue_head(&globalfifo_devp->w_wait);
   return 0;

   fail_malloc: unregister_chrdev_region(devno, 1);
   return result;
 }

 /*模块卸载函数*/
static  void globalmem_exit(void)
 {
   cdev_del(&globalfifo_devp->cdev);   /*注销cdev*/
   kfree(globalfifo_devp);     /*释放设备结构体内存*/
   unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); 
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");
module_param(globalmem_major, int, S_IRUGO);
module_init(globalmem_init);
module_exit(globalmem_exit);
