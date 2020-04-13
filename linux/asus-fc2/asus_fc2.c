#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>

#define ASUS_FC2_VERSION "0.0.20200412"


#define ASUS_FC2_DEVICE_ADDR 0x2a
#define ASUS_FC2_DEVICE_ADDR_BACKUP 0x68 // Is apparently exactly the same as 0x2a

static const unsigned short normal_i2c[] = {
    ASUS_FC2_DEVICE_ADDR, I2C_CLIENT_END };

static const struct i2c_device_id asus_fc2_id[] = {
    { "it8915", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, asus_fc2_id);

static int asus_fc2_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("probe\n");
    return 0;
}

static struct i2c_driver asus_fc2_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name= "asus-fc2",
    },
    .probe = asus_fc2_probe,
    .id_table = asus_fc2_id,
};

module_i2c_driver(asus_fc2_driver);

MODULE_AUTHOR("Renzo <shittydriver@renzokuken.eu>");
MODULE_DESCRIPTION("ASUS FanConnect driver");
MODULE_LICENSE("GPL");
