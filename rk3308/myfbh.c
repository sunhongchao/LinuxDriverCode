
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


struct spilcd_data{   
	int dc_pin;
	int reset_pin;
	int cs_pin;
	int clk_pin;
	int rx_pin;
	int tx_pin;
};
u8 *colorbuf;
struct spilcd{
	 struct spi_device *spidev;
	 struct fb_info *fbi;
	  struct spilcd_data *data;
	  struct delayed_work delayed_worker;
};
struct spilcd  *spilcd;  
static struct fb_ops spilcd_fb_ops = {
    .owner      = THIS_MODULE,
};



//所谓的命令实际上就是寄存器地址
static void spilcd_WriteCmd(unsigned char cmd)
{
	gpio_set_value(spilcd->data->dc_pin, 0);
    spi_write(spilcd->spidev, &cmd, 1);
	gpio_set_value(spilcd->data->dc_pin,1);
	
}

static void spilcd_WriteData(unsigned char dat)
{
    
	gpio_set_value(spilcd->data->dc_pin, 1);
	spi_write(spilcd->spidev, &dat, 1);
	gpio_set_value(spilcd->data->dc_pin, 0);
}

static void spilcd_init(struct spi_device *spi)
{	
	msleep(50);
	spilcd_WriteCmd(0x01); //soft reset
	msleep(50);

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
	
	spilcd_WriteCmd(0x36);    
	//spilcd_WriteData(0x00);//从左到右，从上到下，RGB顺序。
	spilcd_WriteData((1<<5)|(0<<6)|(1<<7)|(1<<3));
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

static struct spilcd_data *spilcd_probe_dt(struct device *dev)
{
	struct spilcd_data *pdata;
	struct device_node *node = dev->of_node;
	
	enum of_gpio_flags of_flags;
	if (!node) {
		return ERR_PTR(-EINVAL);
	}
	pdata = kzalloc(sizeof(struct spilcd_data), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	if (of_find_property(node, "dc-gpios", NULL)) {
		pdata->dc_pin= of_get_named_gpio_flags(node, "dc-gpios", 0, &of_flags);
	}
	if (of_find_property(node, "reset-gpios", NULL)) {
		pdata->reset_pin= of_get_named_gpio_flags(node, "reset-gpios", 0, &of_flags);
	}

	gpio_direction_output(pdata->dc_pin, 1);
	gpio_direction_output(pdata->reset_pin, 1);
	msleep(50);
   	gpio_set_value(pdata->reset_pin,0);
	msleep(200);
	gpio_set_value(pdata->reset_pin,1);
	msleep(100);
	return pdata;
}
static struct fb_info *spilcd_fb_init(void)
{
	struct fb_info *fbi;
    fbi = framebuffer_alloc(0, NULL);
 
    strcpy(fbi->fix.id, "spilcd");
    fbi->fix.smem_len = 240*320*16/8;
    fbi->fix.type     = FB_TYPE_PACKED_PIXELS;
    fbi->fix.visual   = FB_VISUAL_TRUECOLOR; 
    fbi->fix.line_length = 320*2;
     
    fbi->var.xres           = 320;
    fbi->var.yres           = 240;
    fbi->var.xres_virtual   = 320;
    fbi->var.yres_virtual   = 240;
 	fbi->var.bits_per_pixel = 16;
	fbi->var.red.offset     = 11;
    fbi->var.red.length     = 5;
    fbi->var.green.offset   = 5;
    fbi->var.green.length   = 6;
    fbi->var.blue.offset    = 0;
    fbi->var.blue.length    = 5;
	

    fbi->var.activate       = FB_ACTIVATE_NOW;
	fbi->fbops              = &spilcd_fb_ops;
    fbi->screen_size   = 240*320*16/8;  
    fbi->screen_base = dma_alloc_writecombine(NULL,fbi->fix.smem_len, (dma_addr_t *)&fbi->fix.smem_start, GFP_KERNEL);
    register_framebuffer(fbi);  
	return fbi;
}

void set_fbdata(void)
{
	u16 *buf;
	int x, y;
	static u8  count=0;
    u16 color0 = 0x001f; // RGB565, blue    
    u16 color1 = 0xf800; // red
    u16 color2 = 0x07e0; // green
    //u16 color3 = 0xffff; // white
	u16 color3 = ((211>>3)<<11)|((31>>2)<<5)|(207>>3);// white
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

void show_fb(struct fb_info *fbi, struct spi_device *spi)
{
  
   
	/*DMA模式 now work ok*/
	
	u32  len;
	u8 tmp;
	u32 i;

	
	len=fbi->fix.smem_len;
	if(colorbuf==NULL) 
		colorbuf=kzalloc(len,GFP_KERNEL);
	memcpy(colorbuf,fbi->screen_base,len);
	for(i=0;i<len;i=i+2) 
	{
		tmp=colorbuf[i];
		colorbuf[i]=colorbuf[i+1];
		colorbuf[i+1]=tmp;
		
	}
	spilcd_WriteCmd(0x2A); 
	spilcd_WriteData(0);
	spilcd_WriteData(0); 	
	spilcd_WriteData(0X01); 
	spilcd_WriteData(0X3F); 			 
	spilcd_WriteCmd(0X2B); 
	spilcd_WriteData(0);
	spilcd_WriteData(0);
	spilcd_WriteData(0);
	spilcd_WriteData(0Xef);
	spilcd_WriteCmd(0X2C);
	gpio_set_value(spilcd->data->dc_pin, 1);
   	spi_write(spilcd->spidev,(u8*)colorbuf,len);
	gpio_set_value(spilcd->data->dc_pin, 0);
	
	
}

static void delayed_work_handler(struct work_struct *work)
{
	  	//set_fbdata();
	   show_fb(spilcd->fbi,spilcd->spidev);
	   schedule_delayed_work(&spilcd->delayed_worker,HZ/30);
}
static int  spi_lcd_probe(struct spi_device *spi)
{
	struct spilcd_data *pdata;
	struct fb_info *fbi;
	struct device *dev;
	
	dev=&spi->dev;
	pdata = dev->platform_data; 
	if (!pdata) {
		pdata=spilcd_probe_dt(dev);
		if (IS_ERR(pdata))
			{
				goto error;
			}
	}	
	
	spilcd=kzalloc(sizeof(struct spilcd), GFP_KERNEL);
	if (!spilcd)
		{
			goto error;
		}
	spilcd->spidev=spi;
	spilcd->data=pdata; 
	spilcd_init(spi);
	fbi=spilcd_fb_init();
	spilcd->fbi=fbi;
	INIT_DELAYED_WORK(&spilcd->delayed_worker,delayed_work_handler);
	schedule_delayed_work(&spilcd->delayed_worker,HZ/30);
error:
    return 0;
   
}

static int  spi_lcd_remove(struct spi_device *spi)
{
	struct fb_info *fbi;
	 fbi= spilcd->fbi;
	cancel_delayed_work_sync(&spilcd->delayed_worker);
	unregister_framebuffer(fbi);
	dma_free_writecombine(NULL, fbi->fix.smem_len, fbi->screen_base, fbi->fix.smem_start);
	framebuffer_release(fbi);
	kfree(spilcd->data);
	kfree(spilcd);
	kfree(colorbuf);
	return 0;
}


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




