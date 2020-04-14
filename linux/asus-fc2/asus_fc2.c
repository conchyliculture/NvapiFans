#include <linux/module.h>
#include <linux/kernel.h> // for KERN_INFO
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/regmap.h>

#define ASUS_FC2_VERSION "0.0.20200412"


#define ASUS_FC2_DEVICE_ADDR 0x2a
#define ASUS_FC2_DEVICE_ADDR_BACKUP 0x68 // Is apparently exactly the same as 0x2a

#define ASUS_FC2_ID_LOW_REG_ADDR 0x20
#define ASUS_FC2_ID_HIGH_REG_ADDR 0x21

#define ASUS_FC2_IT8915_LOW_ID 0x15
#define ASUS_FC2_IT8915_HIGH_ID 0x89

static const unsigned short normal_i2c[] = {
    ASUS_FC2_DEVICE_ADDR, I2C_CLIENT_END };

static const struct i2c_device_id asus_fc2_id[] = {
    { "asus_fc2", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, asus_fc2_id);


// Describe permissions for each attribute
static umode_t asus_fc2_is_visible(const void *data, enum hwmon_sensor_types type,
				 u32 attr, int channel)
{
	switch (type) {
	case hwmon_pwm:
		switch (attr) {
		case hwmon_temp_input:
		case hwmon_temp_fault:
			return 0444;
		case hwmon_temp_offset:
			return 0644;
		}
		break;
	default:
		break;
	}
	return 0;
}

// hwmon config info
static const struct hwmon_ops asus_fc2_ops = {
	.is_visible = asus_fc2_is_visible,
	.read = asus_fc2_read,
	.write = asus_fc2_write,
};

static const struct hwmon_chip_info asus_fc2_chip_info = {
	.ops = &asus_fc2_ops,
	.info = asus_fc2_info,
};

// Define size of i2c registers & values to 1 byte
static const struct regmap_config asus_fc2_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int asus_fc2_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device *hwmon_dev;
	struct regmap *regmap;
	unsigned int regval_low;
	unsigned int regval_high;
	int ret;

    dev = &client->dev;

	regmap = devm_regmap_init_i2c(client, &asus_fc2_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(dev, "failed to allocate register map\n");
		return PTR_ERR(regmap);
	}

    // Detect chip ID
    ret = regmap_read(regmap, ASUS_FC2_ID_LOW_REG_ADDR, &regval_low);
    if (ret < 0)
        return ret;
    ret = regmap_read(regmap, ASUS_FC2_ID_HIGH_REG_ADDR, &regval_high);
    if (ret < 0)
        return ret;

    if (regval_low != ASUS_FC2_IT8915_LOW_ID) {
        return ENODEV;
    }
    if (regval_high != ASUS_FC2_IT8915_HIGH_ID) {
        return ENODEV;
    }

	i2c_set_clientdata(client, regmap);

	hwmon_dev = devm_hwmon_device_register_with_info(dev,
							 client->name,
							 regmap,
							 &asus_fc2_chip_info,
							 NULL);
	return PTR_ERR_OR_ZERO(hwmon_dev);
}

static struct i2c_driver asus_fc2_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name= "asus_fc2",
    },
    .probe = asus_fc2_probe,
    .id_table = asus_fc2_id,
};
module_i2c_driver(asus_fc2_driver);


MODULE_AUTHOR("Renzo <shittydriver@renzokuken.eu>");
MODULE_DESCRIPTION("ASUS FanConnect driver");
MODULE_LICENSE("GPL");
