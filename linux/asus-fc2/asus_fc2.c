/*
 * Simple hwmon module to control fans plugged onto Asus Fancontrol headers
 *
 * Some useful doc:
 * https://www.kernel.org/doc/html/latest/i2c/index.html
 * https://www.kernel.org/doc/html/latest/i2c/writing-clients.html
 * https://www.kernel.org/doc/html/v5.5/driver-api/i2c.html
 *
 */
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
#define ASUS_FC2_FAN_PWM_REG_ADDR 0x41

#define ASUS_FC2_IT8915_LOW_ID 0x15
#define ASUS_FC2_IT8915_HIGH_ID 0x89

static const unsigned short normal_i2c[] = {
    ASUS_FC2_DEVICE_ADDR, I2C_CLIENT_END };

static const struct i2c_device_id asus_fc2_id[] = {
    { "asus_fc2", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, asus_fc2_id);


static int get_pwm_speed(struct regmap *regmap, long *val){
    int ret;
    unsigned int regval;
	ret = regmap_read(regmap, ASUS_FC2_FAN_PWM_REG_ADDR, &regval);
    *val = regval;
	return ret;
}

static int set_pwm_speed(struct regmap *regmap, long val){
	return regmap_write(regmap, ASUS_FC2_FAN_PWM_REG_ADDR, val);
}

// Describe permissions for each attribute
static umode_t asus_fc2_is_visible(const void *data, enum hwmon_sensor_types type,
				 u32 attr, int channel)
{
	switch (type) {
	case hwmon_pwm:
		switch (attr) {
		case hwmon_pwm_input:
            return 0644;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int asus_fc2_read(struct device *dev, enum hwmon_sensor_types type, u32 attr, int channel, long *val) {
	struct regmap *regmap = dev_get_drvdata(dev);
    switch (attr) {
    case hwmon_pwm_input:
        return get_pwm_speed(regmap, val);
    default:
        return -EOPNOTSUPP;

    }
}

static int asus_fc2_write(struct device *dev, enum hwmon_sensor_types type, u32 attr, int channel, long val) {
	struct regmap *regmap = dev_get_drvdata(dev);
    switch (attr) {
    case hwmon_pwm_input:
        return set_pwm_speed(regmap, val);
    default:
        return -EOPNOTSUPP;

    }
}


// hwmon config info
static const struct hwmon_ops asus_fc2_ops = {
	.is_visible = asus_fc2_is_visible,
	.read = asus_fc2_read,
	.write = asus_fc2_write,
};

static const struct hwmon_channel_info *asus_fc2_info[] = {
	HWMON_CHANNEL_INFO(pwm,
			   HWMON_PWM_INPUT),
//	HWMON_CHANNEL_INFO(fan,
//			   HWMON_F_INPUT)
	NULL
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
        return -ENODEV;
    }
    if (regval_high != ASUS_FC2_IT8915_HIGH_ID) {
        return -ENODEV;
    }

	i2c_set_clientdata(client, regmap);

	hwmon_dev = devm_hwmon_device_register_with_info(dev,
							 client->name,
							 regmap,
							 &asus_fc2_chip_info,
							 NULL);
	return PTR_ERR_OR_ZERO(hwmon_dev);
}

/* Return 0 if detection is successful, -ENODEV otherwise */
  static int asus_fc2_detect(struct i2c_client *new_client, struct i2c_board_info *info) {
      struct i2c_adapter *adapter = new_client->adapter;
      unsigned int regval_low;
      unsigned int regval_high;

      if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
            return -ENODEV;
      }

      regval_low = i2c_smbus_read_byte_data(new_client, ASUS_FC2_ID_LOW_REG_ADDR);
      regval_high = i2c_smbus_read_byte_data(new_client, ASUS_FC2_ID_HIGH_REG_ADDR);

      if (regval_low != ASUS_FC2_IT8915_LOW_ID) {
//        printk(KERN_INFO "bad2 %d!=%d", regval_low, ASUS_FC2_IT8915_LOW_ID);
          return -ENODEV;
      }
      if (regval_high != ASUS_FC2_IT8915_HIGH_ID) {
//        printk(KERN_INFO "bad3 %d!=%d", regval_high, ASUS_FC2_IT8915_HIGH_ID);
          return -ENODEV;
      }
      strlcpy(info->type, "asus_fc2", I2C_NAME_SIZE);
      return 0;
  }

static struct i2c_driver asus_fc2_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name= "asus_fc2",
    },
    .probe = asus_fc2_probe,
    .detect = asus_fc2_detect,
    .address_list = normal_i2c,
    .id_table = asus_fc2_id,
};
module_i2c_driver(asus_fc2_driver);


MODULE_AUTHOR("Renzo <shittydriver@renzokuken.eu>");
MODULE_DESCRIPTION("ASUS FanConnect driver");
MODULE_LICENSE("GPL");
