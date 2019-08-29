/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 * Copyright 2010, 2011 David Jander <david@protonic.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include<linux/uaccess.h>
#include<linux/mutex.h>
//读IO口定时时间 100/10×10ms=100ms
#define DELAY1   HZ/10   
#define DELAY2   HZ*2        
		
struct infrared_data {   
	int rx_pin;          //检测IO口
	int rx_flags;        //设备树中设置的高电平代表有人还是低电平代表有人
};
struct infrared {
	struct platform_device *pdev; 
	int major;
	struct  class *class; 
	struct  timer_list timer;;
	unsigned char human_flag;	//有人标志 1代表有人，0代表没有人		
    struct infrared_data *data;
	struct fasync_struct *async_queue;
	int count;
	struct mutex mutex;
};
static struct infrared  *infrared; 
//传感器只有在人来和人走时候都会给出一个高电平信号，现在只要检测到高电平就发送信号。
static void infrared_timer_func(unsigned long data)
{
	//连续3次检查io口，如果都是一样则认为没有误报
	static  unsigned char new_val[3],count;
	unsigned char val_sum;
	int pin_val;//io口电平
	printk("infrared_timer_func\n");
	mod_timer(&infrared->timer,jiffies + DELAY1);
	pin_val = gpio_get_value(infrared->data->rx_pin);
	//pin_val = 1;
	if(pin_val == 0)
		new_val[count] = 0;
	else if(pin_val == 1)
		new_val[count] = 1;
	count++;
	//printk("pin_val:%u",pin_val);
	if(count >= 3){
		val_sum = new_val[0]+new_val[1]+new_val[2];
		//printk("count:%u,val_sum:%u\n",count,val_sum);
		if(val_sum == 3){
			if(infrared->async_queue == NULL) {
				printk("error:infrared->async_queue = NULL\n");
				//msleep(100);
			}
			if(infrared->async_queue){
				kill_fasync(&infrared->async_queue,SIGIO,POLL_IN);
				mod_timer(&infrared->timer,jiffies + DELAY2);
			}
		}
		count = 0;
		memset(new_val,0,3);
		
	}
	

}
int infrared_open(struct inode *inode, struct file *filp)
{
	    mutex_lock(&infrared->mutex);
		infrared->count++;
		if(infrared->count == 1 ){
			init_timer(&infrared->timer);
			infrared->timer.function = infrared_timer_func;
			infrared->timer.expires = jiffies + DELAY1;
			add_timer(&infrared->timer);
		}
		mutex_unlock(&infrared->mutex);

   return 0; 
}

static int infrared_fasync(int fd,struct file *filp,int mode)
{
	return fasync_helper(fd,filp,mode,&infrared->async_queue);
}
int infrared_release(struct inode *inode, struct file *filp)
{
	mutex_lock(&infrared->mutex);
	infrared->count--;
	if(infrared->count == 0){		
		infrared_fasync(-1,filp,0);
		del_timer_sync(&infrared->timer);
	}
	mutex_unlock(&infrared->mutex);
    return 0;
}

static const struct file_operations infrared_fops = {
	.owner =	THIS_MODULE,
	.open =		infrared_open,
	.fasync=   infrared_fasync,
	.release = infrared_release,
};
static struct infrared_data *infrared_probe_dt(struct device *dev)
{
	struct infrared_data *pdata = NULL;
	struct device_node *np = dev->of_node;
	enum of_gpio_flags of_flags = OF_GPIO_ACTIVE_LOW ;	
	if (!np) {
		return ERR_PTR(-EINVAL);
	}
	pdata = devm_kzalloc(dev,sizeof(struct infrared_data), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	if (of_find_property(np, "rx-gpios", NULL)) {
		pdata->rx_pin = of_get_named_gpio_flags(np, "rx-gpios", 0, &of_flags);
	}
	gpio_direction_input(pdata->rx_pin);
	return pdata;
}
static int infrared_probe(struct platform_device *pdev)
{
	
	struct infrared_data *pdata;
	struct device *dev;
	int ret;
	printk("probe infrared device\n");
	dev=&pdev->dev;
	pdata = dev->platform_data; 
	if(!pdata) {
		pdata=infrared_probe_dt(dev);
		if (IS_ERR(pdata)){
			ret = PTR_ERR(pdata);
				return ret;
		}
	}	
	infrared=devm_kzalloc(dev,sizeof(struct infrared), GFP_KERNEL);
	if (!infrared){
		return -ENOMEM;
	}
	infrared->data = pdata; 
	mutex_init(&infrared->mutex);
	infrared->count = 0;
	infrared->major = register_chrdev(0, "infrared", &infrared_fops);
	infrared->class = class_create(THIS_MODULE, "infrared");	
	device_create(infrared->class, NULL, MKDEV(infrared->major, 0), NULL, "infrared");
	return 0;
}

static int infrared_remove(struct platform_device *pdev)
{
	
	del_timer_sync(&infrared->timer);
	device_destroy(infrared->class, MKDEV(infrared->major, 0));
	class_destroy(infrared->class);
	unregister_chrdev(infrared->major, "infrared");
	return 0;
}

static const struct of_device_id infrared_of_match[] = {
	{ .compatible = "infrared-switch", },
	{ },
};
	
MODULE_DEVICE_TABLE(of,infrared_of_match);

static struct platform_driver infrared_driver = {
	.probe		= infrared_probe,
	.remove		= infrared_remove,
	.driver		= {
		.name	= "infrared-switch",
		.of_match_table = of_match_ptr(infrared_of_match),
	}
};

static int __init infrared_init(void)
{
	return platform_driver_register(&infrared_driver);
}

static void __exit infrared_exit(void)
{
	platform_driver_unregister(&infrared_driver);
}

module_init(infrared_init);
module_exit(infrared_exit);
MODULE_DESCRIPTION("infrared switch Driver");
MODULE_LICENSE("GPL");



