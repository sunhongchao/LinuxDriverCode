
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <sound/core.h>
#include <linux/spi/spi.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>

//static uint32_t pseudo_palette[16];
////从设备树获取的数据
struct spilcd_data{   
	int dc_pin;
	int reset_pin;
};

struct spilcd{
	 struct spi_device *spidev;//spi设备
	 struct fb_info *fbi;//帧缓冲区
	  struct spilcd_data *data;//从设备树获取的数据
	  struct delayed_work delayed_worker;//延时工作队列
};
//全局指针，指向lcd设备
struct spilcd  *spilcd;  
static struct fb_ops spilcd_fb_ops = {
    .owner      = THIS_MODULE,
};

//所谓的命令实际上就是寄存器地址
static void spilcd_WriteCmd(unsigned char cmd)
{
	gpio_set_value(spilcd->data->dc_pin, 0);//命令数据线dc为低电平表示发送的是命令
    spi_write(spilcd->spidev, &cmd, 1);
	gpio_set_value(spilcd->data->dc_pin,1);//命令发送完毕
	
}
static void spilcd_WriteData(unsigned char dat)
{
   
	gpio_set_value(spilcd->data->dc_pin, 1);//命令数据线dc为高电平表示发送的是数据
    spi_write(spilcd->spidev, &dat, 1);
	gpio_set_value(spilcd->data->dc_pin, 0);
}
//读取Ili9341的id，该函数没有测试成功
void SPILcdReadID(void)  
{  
    unsigned char rx_buf[4];  
	spilcd_WriteCmd(0xD3);
	gpio_set_value(spilcd->data->dc_pin, 1);
	ndelay(100);
	spi_read(spilcd->spidev, rx_buf, 4);
	printk("id:%d %d %d %d\n",rx_buf[0],rx_buf[1],rx_buf[2],rx_buf[3]);
	msleep(1000);
}  
//Lcd屏的初始化函数
static void spilcd_init(struct spi_device *spi)
{	
	msleep(50);
	spilcd_WriteCmd(0x01); //soft reset
	msleep(50);
	SPILcdReadID();
	spilcd_WriteCmd(0x28);  //display off
	spilcd_WriteCmd(0xCF); // 功耗控制器，貌似没有用,EXTC引脚高才能使用
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0xC1); 
	spilcd_WriteData(0X30); 
	spilcd_WriteCmd(0xED);  //电源序列控制EXTC引脚高才能使用
	spilcd_WriteData(0x64); 
	spilcd_WriteData(0x03); 
	spilcd_WriteData(0X12); 
	spilcd_WriteData(0X81); 
	spilcd_WriteCmd(0xE8);  //驱动时序控制AEXTC引脚高才能使用
	spilcd_WriteData(0x85); 
	spilcd_WriteData(0x10); 
	spilcd_WriteData(0x7A); 
	spilcd_WriteCmd(0xCB); // 功耗控制器A
	spilcd_WriteData(0x39); 
	spilcd_WriteData(0x2C); 
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x34); 
	spilcd_WriteData(0x02); 
	spilcd_WriteCmd(0xF7); //泵比控制 
	spilcd_WriteData(0x20); 
	spilcd_WriteCmd(0xEA);  //驱动时序控制B
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x00); 
	spilcd_WriteCmd(0xC0);    //功耗控制1
	spilcd_WriteData(0x1B);   //VRH[5:0] 
	spilcd_WriteCmd(0xC1);    //功耗控制2 
	spilcd_WriteData(0x01);   //SAP[2:0];BT[3:0] 
	spilcd_WriteCmd(0xC5);    //VCOM电压控制1
	spilcd_WriteData(0x30); 	 
	spilcd_WriteData(0x30); 	 
	spilcd_WriteCmd(0xC7);    //VCOM 电压控制2 
	spilcd_WriteData(0XB7); 
	/*LCD的扫描方向，感觉应该设置为从左到右，从上到下合适。
	是否可以利用该功能翻转图像?*/
	spilcd_WriteCmd(0x36);    
	//spilcd_WriteData(0x48); 0x48 意味着从右到左，从上到下。BGR顺序
	spilcd_WriteData(0x00);//从左到右，从上到下，RGB顺序。
	spilcd_WriteCmd(0x3A);   
	spilcd_WriteData(0x55);  //设置颜色为16位色
	spilcd_WriteCmd(0xB1);   
	spilcd_WriteData(0x00);   
	spilcd_WriteData(0x1A); 
	spilcd_WriteCmd(0xB6);    // Display Function Control 
	spilcd_WriteData(0x0A); 
	spilcd_WriteData(0xA2); 
	spilcd_WriteCmd(0xF2);    // 3Gamma Function Disable 
	spilcd_WriteData(0x00); 
	spilcd_WriteCmd(0x26);    //Gamma curve selected 
	spilcd_WriteData(0x01); 
	spilcd_WriteCmd(0xE0);    //Set Gamma 
	spilcd_WriteData(0x0F); 
	spilcd_WriteData(0x2A); 
	spilcd_WriteData(0x28); 
	spilcd_WriteData(0x08); 
	spilcd_WriteData(0x0E); 
	spilcd_WriteData(0x08); 
	spilcd_WriteData(0x54); 
	spilcd_WriteData(0XA9); 
	spilcd_WriteData(0x43); 
	spilcd_WriteData(0x0A); 
	spilcd_WriteData(0x0F); 
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x00); 		 
	spilcd_WriteCmd(0XE1);    //Set Gamma 
	spilcd_WriteData(0x00); 
	spilcd_WriteData(0x15); 
	spilcd_WriteData(0x17); 
	spilcd_WriteData(0x07); 
	spilcd_WriteData(0x11); 
	spilcd_WriteData(0x06); 
	spilcd_WriteData(0x2B); 
	spilcd_WriteData(0x56); 
	spilcd_WriteData(0x3C); 
	spilcd_WriteData(0x05); 
	spilcd_WriteData(0x10); 
	spilcd_WriteData(0x0F); 
	spilcd_WriteData(0x3F); 
	spilcd_WriteData(0x3F); 
	spilcd_WriteData(0x0F); 
	spilcd_WriteCmd(0x2B); 
	spilcd_WriteData(0x00);
	spilcd_WriteData(0x00);
	spilcd_WriteData(0x01);
	spilcd_WriteData(0x3f);
	spilcd_WriteCmd(0x2A); 
	spilcd_WriteData(0x00);
	spilcd_WriteData(0x00);
	spilcd_WriteData(0x00);
	spilcd_WriteData(0xef);	 
	spilcd_WriteCmd(0x11);
	msleep(120);
	spilcd_WriteCmd(0x29); 

	
} 
//从设备树中提取数据dev=&spi->dev;
static struct spilcd_data *spilcd_probe_dt(struct device *dev)
{
	struct spilcd_data *pdata;
	struct device_node *node = dev->of_node;//指向子节点设备树的头
	enum of_gpio_flags of_flags;//用来获取io口的配置，是输入还是输出。
	if (!node) {
		printk("no  node found\n");
		return ERR_PTR(-EINVAL);
	}
	//pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	pdata = kzalloc(sizeof(struct spilcd_data), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	//从设备树中查找dc-gpios项，如果找到就提取io口的编号和设置标志。
	//pdata->dc_pin就是io口编号，每一个io口都有一个单独的编号。
	if (of_find_property(node, "dc-gpios", NULL)) {
		pdata->dc_pin= of_get_named_gpio_flags(node, "dc-gpios", 0, &of_flags);
	}
	if (of_find_property(node, "reset-gpios", NULL)) {
		pdata->reset_pin= of_get_named_gpio_flags(node, "reset-gpios", 0, &of_flags);
	}
	//配置dc_pin引脚为输出引脚，并且初始化输出电平为高
	gpio_direction_output(pdata->dc_pin, 1);
	//配置reset_pin引脚为输出引脚，并且初始化输出电平为高.ili9431复位电平为低电平。
	//因此最终电平应该设置为高电平。
	gpio_direction_output(pdata->reset_pin, 1);
	msleep(200);
   	gpio_set_value(pdata->reset_pin,0);//复位Ili9431
	msleep(200);
	gpio_set_value(pdata->reset_pin,1);
	msleep(200);
	printk("reset pin is %d",pdata->reset_pin);
	return pdata;
}
static struct fb_info *spilcd_fb_init(void)
{
	struct fb_info *fbi;
    fbi = framebuffer_alloc(0, NULL);//分配帧缓冲区
 
    strcpy(fbi->fix.id, "spilcd");
    fbi->fix.smem_len = 240*320*16/8;
    fbi->fix.type     = FB_TYPE_PACKED_PIXELS;//像素存储类型
    fbi->fix.visual   = FB_VISUAL_TRUECOLOR;//颜色是真彩色
    fbi->fix.line_length = 240*2;//每行数据子节数
     
    fbi->var.xres           = 240;//x轴分辨率
    fbi->var.yres           = 320;//y轴分辨率
    fbi->var.xres_virtual   = 240;
    fbi->var.yres_virtual   = 320;
 	fbi->var.bits_per_pixel = 16;//每个像素占的位数
	fbi->var.red.offset     = 11; //红色在16位中起始位
    fbi->var.red.length     = 5;//红色占用16位中几位
    fbi->var.green.offset   = 5;
    fbi->var.green.length   = 6;
    fbi->var.blue.offset    = 0;
    fbi->var.blue.length    = 5;
	

    fbi->var.activate       = FB_ACTIVATE_NOW;//设置完成立即生效
	fbi->fbops              = &spilcd_fb_ops;//自定义的操作fb的函数
	//fbi->pseudo_palette = pseudo_palette;
    fbi->screen_size   = 240*320*16/8;  //一帧颜色数据占用的子节数
	//申请帧缓冲区，采用	DMA模式
   fbi->screen_base = dma_alloc_writecombine(NULL,fbi->fix.smem_len, (dma_addr_t *)&fbi->fix.smem_start, GFP_KERNEL);
    register_framebuffer(fbi);//注册帧缓冲区
	return fbi;
}
//显示测试函数，最终代码不适用该函数，直接向lcd发数据，没有采用framebuffer
void show_fb_test(void)
{
  	int x, y;
    u16 color0 = 0x001f; // RGB565, blue    
    u16 color1 = 0xf800; // red
    u16 color2 = 0x07e0; // green
    u16 color3 = 0xffff; // white
    u16 color;
    spilcd_WriteCmd(0x2A); //设置写显存x轴起始坐标
	spilcd_WriteData(0);
	spilcd_WriteData(0); 			 
	spilcd_WriteCmd(0X2B); //设置写显存y轴起始坐标
	spilcd_WriteData(0);
	spilcd_WriteData(0);
	spilcd_WriteCmd(0X2C);//开始写显存，数据会从（x,y）所在的显存开始存储数据
   for (y = 0; y < 320; y++)
    {
        for (x = 0; x < 240; x++)
        {
            if (x < 120)
                color = (y < 160) ? color0 : color1; 
            else
                color = (y < 160) ? color2 : color3; 

            spilcd_WriteData(color >> 8);
            spilcd_WriteData(color & 0xff);
        }
    }
		 
}
//设置缓冲区数据，测试framebuffer工作是否正常。显示屏会依次全屏显示蓝色，红色，绿色，白色
//最终代码不使用该函数
void set_fbdata(void)
{
	u16 *buf;
	int x, y;
	static u8  count=0;
    u16 color0 = 0x001f; // RGB565, blue    
    u16 color1 = 0xf800; // red
    u16 color2 = 0x07e0; // green
    u16 color3 = 0xffff; // white
	u16 color=0;
	struct fb_info *fbi=spilcd->fbi;
    if(count%4==0)
		color=color0;
	else if(count%4==1)
		color=color1;
	else if(count%4==2)
		color=color2;
	else if(count%4==3)
		color=color3;
	count++;
	buf= (u16*)(fbi->screen_base);//内存基地址
	for (y = 0; y < fbi->var.yres; y++)
    {
        for (x = 0; x < fbi->var.xres; x++)
        {
  		    buf[y*fbi->var.xres+x]= color;//set像素点的16位数据
        }
    }
	//printk("current color%d\n",color);
}
//显示函数
void show_fb(struct fb_info *fbi, struct spi_device *spi)
{
  
   
	/*DMA模式 now work ok*/
	u8 *buf;
	u32  len;
	u8 tmp;
	u32 i;
	printk("delayed_work show\n");	
	buf= (u8*)(fbi->screen_base);//缓存基地址
	len=fbi->fix.smem_len;//缓存长度
	printk("len:%d,buf:%p\n",len,buf);
	/*
	由于默认的缓存中颜色数据是低字节在前，高子节在后。而spi lcd屏幕只能一位一位的发送。而且要求高位
	在前。为了使用DMA模式，一次把所有数据都传送给spi控制器。把高低字节交换。交换后高字节在前。
	而spi发送时会自动首先发送高位数据。
	*/
	for(i=0;i<len;i=i+2) 
	{
		tmp=buf[i];
		buf[i]=buf[i+1];
		buf[i+1]=tmp;
	}
	printk("color:%d\n ",(u16)(buf[i]<<8|buf[i+1]));
	spilcd_WriteCmd(0x2A); //设置x轴数据起始位置
	spilcd_WriteData(0);
	spilcd_WriteData(0); 			 
	spilcd_WriteCmd(0X2B);  //设置y轴数据起始位置
	spilcd_WriteData(0);
	spilcd_WriteData(0);
	spilcd_WriteCmd(0X2C);//指示开始发送颜色数据，接收到数据存储在（x,y）坐标映射的显存位置
	gpio_set_value(spilcd->data->dc_pin, 1);//高电平指示发送数据
   	spi_write(spilcd->spidev,(u8*)buf,len);
	gpio_set_value(spilcd->data->dc_pin, 0);
	
   

}
//延时工作队列函数，定时时间到执行该函数
static void delayed_work_handler(struct work_struct *work)
{
	   //set_fbdata();
	   show_fb(spilcd->fbi,spilcd->spidev);//发送颜色数据到显示屏
	   schedule_delayed_work(&spilcd->delayed_worker,HZ/30);//HZ/100*100);//每隔30ms刷一次屏幕
}
static int  spi_lcd_probe(struct spi_device *spi)
{
	struct spilcd_data *pdata;
	struct fb_info *fbi;
	struct device *dev;
	printk("probed lcd device node\n");
	dev=&spi->dev;
	pdata = dev->platform_data; //是否有平台数据，设备初始化注册
	if (!pdata) {
		pdata=spilcd_probe_dt(dev);//从设备树获取数据信息
		if (IS_ERR(pdata))
			{
				printk("pdata get error\n");
				goto error;
			}
	}	
	
	//spilcd=devm_kzalloc(dev, sizeof(*spilcd), GFP_KERNEL);
	spilcd=kzalloc(sizeof(struct spilcd), GFP_KERNEL);
	if (!spilcd)
		{
			printk("spilcd get mem file\n");
			goto error;
		}
	spilcd->spidev=spi;
	spilcd->data=pdata; 
	spilcd_init(spi);//初始化lcd屏
	//show_fb_test();
	printk("dc pin is %d\n",pdata->dc_pin);
	fbi=spilcd_fb_init();//申请并注册帧缓冲区
	spilcd->fbi=fbi;
	//初始化延时工作队列
	INIT_DELAYED_WORK(&spilcd->delayed_worker,delayed_work_handler);
	//启动工作队列。延时是以jiffes计数单位，HZ=100就是1S中断100次，每次10ms，
	//即1 jiffes=10ms,设置值HZ/100*100=100*jiffes=100*10ms=1s
	//延时工作队列就是每隔一段时间执行一次队列函数delayed_work_handler
	schedule_delayed_work(&spilcd->delayed_worker,HZ/100*100);
error:
    return 0;
   
}

static int  spi_lcd_remove(struct spi_device *spi)
{
	struct fb_info *fbi;
	printk("spilcd:%p",spilcd);
	 fbi= spilcd->fbi;
	cancel_delayed_work_sync(&spilcd->delayed_worker);//等待本次任务完成后再取消等待队列
	unregister_framebuffer(fbi);//卸载注册的帧缓冲区
	//释放DMA申请的缓冲区
	dma_free_writecombine(NULL, fbi->fix.smem_len, fbi->screen_base, fbi->fix.smem_start);
	framebuffer_release(fbi);//释放帧缓冲区
	kfree(spilcd->data);
	kfree(spilcd);
	return 0;
}

//设备树中compatible名字一样则匹配调用probe函数

static const struct of_device_id dt_ids[] = {							   
	{ .compatible = "ilitek,ili9341", },									   
	{}, 															   
};																		   
										   
MODULE_DEVICE_TABLE(of, dt_ids);		  

																		  
static struct spi_driver spi_lcd_drv = {
	.driver = {
		.name	= "spilcd",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(dt_ids), 
	},
	.probe		= spi_lcd_probe,
	.remove		= spi_lcd_remove,
};

static int __init spi_lcd_init(void)
{
    return spi_register_driver(&spi_lcd_drv);
}

static  void __exit spi_lcd_exit(void)
{
    spi_unregister_driver(&spi_lcd_drv);
}


late_initcall(spi_lcd_init);
module_exit(spi_lcd_exit);
MODULE_DESCRIPTION("SPI lcd Driver");
MODULE_LICENSE("GPL");




