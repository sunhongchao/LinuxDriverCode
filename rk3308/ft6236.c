#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>  

#define FT6236_MAX_TOUCH_POINTS		2
#define FT6236_REG_TH_GROUP		0x80
#define FT6236_REG_PERIODACTIVE		0x88
#define FT6236_REG_LIB_VER_H		0xa1
#define FT6236_REG_LIB_VER_L		0xa2
#define FT6236_REG_CIPHER		0xa3
#define FT6236_REG_FIRMID		0xa6
#define FT6236_REG_FOCALTECH_ID		0xa8
#define FT6236_REG_RELEASE_CODE_ID	0xaf

#define FT6236_EVENT_PRESS_DOWN		0
#define FT6236_EVENT_LIFT_UP		1
#define FT6236_EVENT_CONTACT		2
#define FT6236_EVENT_NO_EVENT		3

struct ft6236_data {
	struct i2c_client *client;
	struct input_dev *input;
	int reset_pin;
	int irq_pin;
	u32 max_x;
	u32 max_y;
	bool invert_x;
	bool invert_y;
	bool swap_xy;
	
};

struct ft6236_touchpoint {
	union {
		u8 xhi;
		u8 event;
	};
	u8 xlo;
	union {
		u8 yhi;
		u8 id;
	};
	u8 ylo;
	u8 weight;
	u8 misc;
} __packed;
struct  point_pos{
	int x;
	int y;
};
/* This packet represents the register map as read from offset 0 */
struct ft6236_packet {
	u8 dev_mode;
	u8 gest_id;
	u8 touches;
	struct ft6236_touchpoint points[FT6236_MAX_TOUCH_POINTS];
} __packed;

static const struct of_device_id ft6236_of_match[] = {
	{ .compatible = "ft6236", },
	{ },
};
MODULE_DEVICE_TABLE(of, ft6236_of_match);
static const struct i2c_device_id ft6236_id[] = {
	{ "ft6236", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ft6236_id);
static int ft6236_read(struct i2c_client *client, u8 reg, u8 len, void *data)
{
	int error;

	error = i2c_smbus_read_i2c_block_data(client, reg, len, data);
	if (error < 0)
		return error;

	if (error != len)
		return -EIO;

	return 0;
}


static irqreturn_t ft6236_interrupt(int irq, void *dev_id)
{
	struct ft6236_data *ft6236 = dev_id;
	struct device *dev = &ft6236->client->dev;
	struct input_dev *input = ft6236->input;
	struct ft6236_packet buf;
	u8 touches;
	static int times;
	int i, error;
	static int pre_x[2],pre_xx[2],pre_y[2],pre_yy[2];
	int now_x[2],now_y[2];
	if((ft6236 == NULL)||(dev == NULL)|| (input == NULL)){
		printk("error:ft6236 = NULL\n");
		//msleep(100);
	}
	
	error = ft6236_read(ft6236->client, 0, sizeof(buf), &buf);
	if (error) {
		dev_err(dev, "read touchdata failed %d\n", error);
		return IRQ_HANDLED;
	}
	touches = buf.touches & 0xf;
	if(touches > FT6236_MAX_TOUCH_POINTS) {
		dev_dbg(dev,
			"%d touch points reported, only %d are supported\n",
			touches, FT6236_MAX_TOUCH_POINTS);
		touches = FT6236_MAX_TOUCH_POINTS;
	}else if(touches <= 0){
		for (i = 0; i < FT6236_MAX_TOUCH_POINTS; i++) {
			pre_xx[i] = 0;
			pre_yy[i] = 0;
 			pre_x[i] = 0;
			pre_y[i] = 0;
		}
		if(times > 3){
			printk(" usual touch key up\n");
			input_mt_sync_frame(input);
			input_sync(input);
		}else {
			printk("unusual touch key up\n");
		}
		times = 0;
		return IRQ_HANDLED;
	}

	for (i = 0; i < touches; i++) {
		struct ft6236_touchpoint *point = &buf.points[i];
		u16 x = ((point->xhi & 0xf) << 8) | buf.points[i].xlo;
		u16 y = ((point->yhi & 0xf) << 8) | buf.points[i].ylo;
		u8 event = point->event >> 6;
		u8 id = point->id >> 4;
		bool act = (event == FT6236_EVENT_PRESS_DOWN ||
			    event == FT6236_EVENT_CONTACT);

		input_mt_slot(input, id);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, act);
		if (!act)
			continue;
		times++;
		ft6236->invert_x=1;
		if (ft6236->invert_x)
			x = ft6236->max_x - x;

		if (ft6236->invert_y)
			y = ft6236->max_y - y;

		ft6236->swap_xy=1;
		if (ft6236->swap_xy){
			now_x[i]=y;
			now_y[i]=x;
		}else {
			now_x[i]=x;
			now_y[i]=y;
		}
	   printk("id:%d pos:%d,%d\n",i,now_x[i],now_y[i]);
		if((pre_x[i] == 0) && (pre_y[i] == 0)){
			pre_x[i]=now_x[i];
			pre_y[i]=now_y[i];
			continue;
		
		} else if((abs(pre_x[i]-now_x[i]) > 8) || ((pre_y[i]-now_y[i]) > 8)){
			pre_x[i]=0; 
			pre_y[i]=0;
			continue;
			
		}
		if((pre_xx[i] != 0) && ((pre_yy[i] != 0))) { 
			input_report_abs(input, ABS_MT_POSITION_X, now_x[i]);
			input_report_abs(input, ABS_MT_POSITION_Y, now_y[i]);
			
		}
	
		pre_xx[i] =  pre_x[i];
		pre_yy[i] =  pre_y[i];
		pre_x[i]  =  now_x[i];
		pre_y[i]  =  now_y[i];
		
	}
	input_mt_sync_frame(input);
	input_sync(input);

	return IRQ_HANDLED;
}
static void ft6236_parse_devtree(struct ft6236_data *ft6236)
{
 	enum of_gpio_flags of_flags;
	enum of_gpio_flags irq_flags;
    struct device_node *np; 
	struct i2c_client *client;
    u32 addr;   
    struct device *dev;
	client=ft6236->client;
	dev = &client->dev;
    np = dev->of_node;  
    of_property_read_u32(np,"reg",&addr);
    client->addr=addr;
	if (of_find_property(np, "reset-gpios", NULL)) {
		ft6236->reset_pin= of_get_named_gpio_flags(np, "reset-gpios", 0, &of_flags);
	}
	ft6236->irq_pin = of_get_named_gpio_flags(np, "interrupts", 0, &irq_flags); 		
	client->irq = gpio_to_irq(ft6236->irq_pin);
	gpio_direction_output(ft6236->reset_pin, 1);
	msleep(200);
   	gpio_set_value(ft6236->reset_pin,0);
	msleep(200);
	gpio_set_value(ft6236->reset_pin,1);
	msleep(200);

}
static int mtp_ft5x0x_i2c_rxdata(struct i2c_client *client, char *rxdata, int length) {
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	

	return ret;
}
static int mtp_ft5x06_valid(struct i2c_client *client)
{
	u8 buf[32] = { 0 };
	int ret;
	
	buf[0] = 0xA8;
	ret = mtp_ft5x0x_i2c_rxdata(client, buf, 1);
	if (ret < 0) {
		return ret;
	}	
	return 0;
}
static int ft6236_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct ft6236_data *ft6236;
	struct input_dev *input;
	u32 fuzz_x = 0, fuzz_y = 0;
	int error;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENXIO;
	ft6236 = devm_kzalloc(dev, sizeof(*ft6236), GFP_KERNEL);
	if (!ft6236)
		return -ENOMEM;	
	ft6236->client = client;
	if (!of_match_device(of_match_ptr(ft6236_of_match),dev)) {  
        dev_err(dev, "Failed to match.\n");  
        return -EINVAL;  
    }    

	ft6236_parse_devtree(ft6236);
	mtp_ft5x06_valid(client);
	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;
	ft6236->input = input;
	input->name = client->name;
	input->id.bustype = BUS_I2C;

	if (device_property_read_u32(dev, "touchscreen-size-x",
				     &ft6236->max_x) ||
	    device_property_read_u32(dev, "touchscreen-size-y",
				     &ft6236->max_y)) {
		dev_err(dev, "touchscreen-size-x and/or -y missing\n");
		return -EINVAL;
	}
	device_property_read_u32(dev, "touchscreen-fuzz-x", &fuzz_x);
	device_property_read_u32(dev, "touchscreen-fuzz-y", &fuzz_y);
	ft6236->invert_x = device_property_read_bool(dev,
						     "touchscreen-inverted-x");
	ft6236->invert_y = device_property_read_bool(dev,
						     "touchscreen-inverted-y");
	ft6236->swap_xy = device_property_read_bool(dev,
						    "touchscreen-swapped-x-y");

	if (ft6236->swap_xy) {
		input_set_abs_params(input, ABS_MT_POSITION_X, 0,
				     ft6236->max_y, fuzz_y, 0);
		input_set_abs_params(input, ABS_MT_POSITION_Y, 0,
				     ft6236->max_x, fuzz_x, 0);
	} else {
		input_set_abs_params(input, ABS_MT_POSITION_X, 0,
				     ft6236->max_x, fuzz_x, 0);
		input_set_abs_params(input, ABS_MT_POSITION_Y, 0,
				     ft6236->max_y, fuzz_y, 0);
	}

	error = input_mt_init_slots(input, FT6236_MAX_TOUCH_POINTS,
				    INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
	if (error)
		return error;

	error = devm_request_threaded_irq(dev, client->irq, NULL,
					  ft6236_interrupt, IRQF_ONESHOT|IRQ_TYPE_EDGE_RISING,
					  client->name, ft6236);
	if (error) {
		dev_err(dev, "request irq %d failed: %d\n", client->irq, error);
		return error;
	}
	error = input_register_device(input);
	if (error) {
		dev_err(dev, "failed to register input device: %d\n", error);
		return error;
	}
	
	return 0;
}
static int  ft6236_remove(struct i2c_client *client)
{    
	
    return 0;
}


static struct i2c_driver ft6236_driver = {
	.driver = {
		.name = "ft6236",
		.of_match_table = of_match_ptr(ft6236_of_match),
	},
	.probe = ft6236_probe,
	.remove     = ft6236_remove,
	.id_table = ft6236_id,
};
module_i2c_driver(ft6236_driver);

MODULE_LICENSE("GPL");
