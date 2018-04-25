This is a first stab at creating a hack to control an ip178g from userspace.
The hack is one of the many you can find on the interwebs: create a sysfs file that gives you simple access to the MDIO bus.

IC Plus corp. has a load of cheap ass switches.
All the switches cary the same phy_id for the ports (register 2 and 3), even though the phy capabilities are not the same.
The naming of the devices are also confusing: the IP175B is newer than the IP175G, the IP175C seems to be much older though.

What is unclear is if the G series do IVL or SVL.

About the hack: I was looking into creating a dsa driver. But it's not really a dsa chip. All the phy's do behave like regular phy's and should be controllable by the generic phy driver and ethtool or mii-tool.
For that dsa structure should actually help, as it provides structure for slave network devices.
