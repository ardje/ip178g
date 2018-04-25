#include <linux/ethtool.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/mdio.h>
#include <linux/device.h>
#include <linux/uaccess.h>



MODULE_DESCRIPTION("userspace phy access for switch control");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("jags & ard");

static u16 data = 0;

/* Supported Device ID Tables */
static struct mdio_device_id icplus_IP178G_id[] = {
    {0x02430d80, 0xfffffc00},
    {}
};
MODULE_DEVICE_TABLE(mdio, icplus_IP178G_id);

static ssize_t switch_conf_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    return sprintf(buf, "%x\n", data);
}

static ssize_t  switch_conf_store(struct device *dev, struct device_attribute *attr,
        const char *buffer, size_t size)
{
    u32 value;
    int phy_id;
    struct phy_device *phydev = to_phy_device(dev);
    u32 reg_num = 0;
    sscanf(buffer, "%x", &value);
    if (value & 0xFF000000) {
        phy_id = (value & 0xFF000000) >> 24;
        reg_num = (value & 0x00FF0000) >> 16;
        data =  (value & 0x0000FFFF);
        mdiobus_write(phydev->mdio.bus, phy_id, reg_num, data);
        /*mdiobus_write(phydev->mdio.bus, phydev->mdio.addr, reg_num, data);*/
    }
    else {
        phy_id = (value & 0xFF00) >> 8;
        reg_num = (value & 0x00FF);
        data = mdiobus_read(phydev->mdio.bus, phy_id, reg_num);
        /*data = mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, reg_num);*/
    }
    return size;
}

static DEVICE_ATTR_RW(switch_conf);

static struct attribute *switch_attrs[] = {
    &dev_attr_switch_conf.attr,
    NULL,
};
ATTRIBUTE_GROUPS(switch);

/*
static struct attribute_group attr_group_switch = {
    .attrs = attrs_switch,
};
*/

/* Initialize the IC Plus IP178G switch access */
static int icplus_IP178G_probe(struct phy_device *phydev)
{
    int err = 0;
    int i;

    pr_debug("mdio/phy sub system detected undetermined IC Plus switch\n");
    pr_debug("switch id is %04x%04x @ %s:%d\n", mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, 0x2), mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, 0x03),phydev->mdio.bus->name,phydev->mdio.addr);
    /* Probe bus signatures for switch */
    /* phy 0..7 have the IC plus id, phy 20..24 should read non 0xffff at reg 2,
       any other phy at the bus indicates an unknown IC Plus switch.
       All phy id's are the same :-(
    */
    err=0;
    for(i=0;i<32;i++) {
	int d=mdiobus_read(phydev->mdio.bus, i, 2);
        if (d != 0xffff) {
          if ((i >7 && i<20) ||i>24 || (i<8 && data != 0x243)) err|=1;
	} else {
          if (i <8 || (i>19 && i<25)) err|=1;
	}
    }

    if(err) {
        printk(KERN_INFO "IC Plus switch not an IP17[58]G type\n");
        //return -1;
    }
    /* Add our file access */
    err = device_add_groups(&phydev->mdio.dev,switch_groups);
    return err;
}

/* PHY Driver */
static struct phy_driver icplus_IP178G_driver = {
    .phy_id     = 0x02430d80,
    .phy_id_mask    = 0xffffffff,
    .name       = "IC Plus IP178G",
    .features   = (PHY_BASIC_FEATURES),
    /*.flags      = PHY_HAS_INTERRUPT,*/
    .probe      = icplus_IP178G_probe,
    .config_aneg    = genphy_config_aneg,
    .read_status    = genphy_read_status,
    /*.driver     = { .owner = THIS_MODULE },*/
};
/*Switch initialize function */

/* Init exit */
static int __init mdio_IP178G_init(void)
{
    return phy_driver_register(&icplus_IP178G_driver,THIS_MODULE);
}

static void __exit mdio_IP178G_exit(void)
{
    phy_driver_unregister(&icplus_IP178G_driver);
}

module_init(mdio_IP178G_init);
module_exit(mdio_IP178G_exit);
