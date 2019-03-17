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

#define GLOBALFIFO_SIZE 0x1000 /*ȫ���ڴ����4KB*/
#define MEM_CLEAR 0x1  /*����ȫ���ڴ�*/
#define GLOBALMEM_MAJOR 251    /*Ԥ���globalmem�����豸��*/

static int globalmem_major = GLOBALMEM_MAJOR;
 /*globalmem�豸�ṹ��*/
struct globalfifo_dev
{
    struct cdev cdev; /*cdev�ṹ��*/
	unsigned int current_len;//fifo length
    unsigned char mem[GLOBALFIFO_SIZE]; /*ȫ���ڴ�*/
	struct semaphore sem;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
};

struct globalfifo_dev *globalfifo_devp; /*�豸�ṹ��ָ��*/
  /*�ļ��򿪺���*/
int globalmem_open(struct inode *inode, struct file *filp)
 {
	/*���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ��*/
    filp->private_data = globalfifo_devp;
   return 0; }
 	/*�ļ��ͷź���*/
int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* ioctl�豸���ƺ���*/
 static int globalmem_ioctl(struct inode *inodep, struct file *filp, 					unsigned int cmd, unsigned long arg)
 {
   struct globalfifo_dev *dev = filp->private_data;/*����豸�ṹ��ָ��*/

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

 /*������*/
static ssize_t globalmem_read(struct file *filp, char __user *buf,
				size_t size,loff_t *ppos)
 {
    
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data; /*����豸�ṹ��ָ��*/
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
	
    /*�ں˿ռ���û��ռ�*/
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

  /*д����*/
static ssize_t globalmem_write(struct file *filp, const char __user 								*buf,size_t size, loff_t *ppos)
{
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data; /*����豸�ṹ��ָ��*/
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

 /* seek�ļ���λ����*/
 static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig) {
   loff_t ret = 0;

   switch (orig)
{
     case 0:   /*����ļ���ʼλ��ƫ��*/
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
    case 1:   /*����ļ���ǰλ��ƫ��*/
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

 /*�ļ������ṹ��*/
 static  struct file_operations globalmem_fops ={
   .owner = THIS_MODULE,
   .llseek = globalmem_llseek,
   .read = globalmem_read,
   .write = globalmem_write,
   .ioctl = globalmem_ioctl,
   .open = globalmem_open,
   .release = globalmem_release,
 };

 /*��ʼ����ע��cdev*/
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

 /*�豸����ģ����غ���*/
static int __init  globalmem_init(void)
 {
  int result=0;
  dev_t devno = MKDEV(globalmem_major, 0);

   /*�����豸��*/
  if (globalmem_major)
     	result = register_chrdev_region(devno, 1, "globalfifo");
   if(result<0) /*��̬�����豸��*/
   {
    	result = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
    	globalmem_major = MAJOR(devno);
   }
   if (result < 0)
    return result;

   /*��̬�����豸�ṹ����ڴ�*/
   globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
   if (!globalfifo_devp)    /*����ʧ��*/
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

 /*ģ��ж�غ���*/
static  void globalmem_exit(void)
 {
   cdev_del(&globalfifo_devp->cdev);   /*ע��cdev*/
   kfree(globalfifo_devp);     /*�ͷ��豸�ṹ���ڴ�*/
   unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); 
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");
module_param(globalmem_major, int, S_IRUGO);
module_init(globalmem_init);
module_exit(globalmem_exit);
