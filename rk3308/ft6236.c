/*
 * FocalTech FT6236 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
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

/*
 * This struct is a touchpoint as stored in hardware.  Note that the id,
 * as well as the event, are stored in the upper nybble of the hi byte.
 */
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

/* This packet represents the register map as read from offset 0 */
struct ft6236_packet {
	u8 dev_mode;
	u8 gest_id;
	u8 touches;
	struct ft6236_touchpoint points[FT6236_MAX_TOUCH_POINTS];
} __packed;
struct ft6236_data *ft6236;
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
	int i, error;
	printk("touch interrupt \n");
	error = ft6236_read(ft6236->client, 0, sizeof(buf), &buf);
	if (error) {
		dev_err(dev, "read touchdata failed %d\n", error);
		return IRQ_HANDLED;
	}

	touches = buf.touches & 0xf;
	if (touches > FT6236_MAX_TOUCH_POINTS) {
		dev_dbg(dev,
			"%d touch points reported, only %d are supported\n",
			touches, FT6236_MAX_TOUCH_POINTS);
		touches = FT6236_MAX_TOUCH_POINTS;
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

		if (ft6236->invert_x)
			x = ft6236->max_x - x;

		if (ft6236->invert_y)
			y = ft6236->max_y - y;

		if (ft6236->swap_xy) {
			input_report_abs(input, ABS_MT_POSITION_X, y);
			input_report_abs(input, ABS_MT_POSITION_Y, x);
		} else {
			input_report_abs(input, ABS_MT_POSITION_X, x);
			input_report_abs(input, ABS_MT_POSITION_Y, y);
			printk("x:%d,y:%d\n",x,y);
		}
	}

	input_mt_sync_frame(input);
	input_sync(input);

	return IRQ_HANDLED;
}

static u8 ft6236_debug_read_byte(struct ft6236_data *ft6236, u8 reg)
{
	struct i2c_client *client = ft6236->client;
	u8 val = 0;
	int error;

	error = ft6236_read(client, reg, 1, &val);
	if (error)
		dev_dbg(&client->dev,
			"error reading register 0x%02x: %d\n", reg, error);

	return val;
}

static void ft6236_debug_info(struct ft6236_data *ft6236)
{
	struct device *dev = &ft6236->client->dev;

	dev_dbg(dev, "Touch threshold is %d\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_TH_GROUP) * 4);
	dev_dbg(dev, "Report rate is %dHz\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_PERIODACTIVE) * 10);
	dev_dbg(dev, "Firmware library version 0x%02x%02x\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_LIB_VER_H),
		ft6236_debug_read_byte(ft6236, FT6236_REG_LIB_VER_L));
	dev_dbg(dev, "Firmware version 0x%02x\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_FIRMID));
	dev_dbg(dev, "Chip vendor ID 0x%02x\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_CIPHER));
	dev_dbg(dev, "CTPM vendor ID 0x%02x\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_FOCALTECH_ID));
	dev_dbg(dev, "Release code version 0x%02x\n",
		ft6236_debug_read_byte(ft6236, FT6236_REG_RELEASE_CODE_ID));
}


static void ft6236_parse_devtree(struct ft6236_data *ft6236)
{
 	enum of_gpio_flags of_flags;
	enum of_gpio_flags irq_flags;
    struct device_node *np; 
	struct i2c_client *client;
    u32 addr;   
    struct property *prop=NULL;
    struct device *dev;
	client=ft6236->client;
	dev = &client->dev;
    np = dev->of_node;  
    of_property_read_u32(np,"reg",&addr);
    client->addr=addr;
    prop=of_find_property(np,"compatible",NULL);
    if(prop) printk("find compatible = %s\n",(char*)prop->value);
	if (of_find_property(np, "reset-gpios", NULL)) {
		ft6236->reset_pin= of_get_named_gpio_flags(np, "reset-gpios", 0, &of_flags);
	}
	ft6236->irq_pin = of_get_named_gpio_flags(np, "interrupts", 0, &irq_flags); 		
	//gpio_request(ft6236->irq_pin, "ft6236"); 
	client->irq = gpio_to_irq(ft6236->irq_pin);
	printk("irq_pin:%d,irq_no%d",ft6236->irq_pin,client->irq);
	gpio_direction_output(ft6236->reset_pin, 1);
	msleep(200);
   	gpio_set_value(ft6236->reset_pin,0);
	msleep(200);
	gpio_set_value(ft6236->reset_pin,1);
	msleep(200);
	printk("reset pin is %d",ft6236->reset_pin);


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
	if (ret < 0)
		pr_err("%s: i2c read error: %d\n", __func__, ret);

	return ret;
}
static int mtp_ft5x06_valid(struct i2c_client *client)
{
	u8 buf[32] = { 0 };
	int ret;
	printk("mtp_ft5x06_valid : addr = 0x%x\n", client->addr);
	/* 进一步判断设备的合法性 */
	buf[0] = 0xA8; /* chip vendor id */	
	ret = mtp_ft5x0x_i2c_rxdata(client, buf, 1);
	if (ret < 0) {
		printk("There is not real device, i2c read err\n");
		return ret;
	}
		printk("find real device\n");
		printk("chip vendor id = 0x%x\n", buf[0]);
	return 0;
}
static int ft6236_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	//struct ft6236_data *ft6236;
	struct input_dev *input;
	u32 fuzz_x = 0, fuzz_y = 0;
	//u8 val;
	int error;
	printk("probe ft6236 device \n");
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
	ft6236_debug_info(ft6236);
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
	printk("register touch device\n");
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
