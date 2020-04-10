# Requirements

apt install i2c-tools

modprobe i2c-detect

Set fan at full speed:
```
i2cset 2 0x2a 0x41 0xFF
```
Change 0xFF for a lower value for lower speeds.

