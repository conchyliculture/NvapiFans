# Requirements

Tested with drivers from NVIDIA (`nvidia` module). Driver version needs to be [>=304.137](https://download.nvidia.com/XFree86/Linux-x86_64/304.137/README/i2c.html)
for the i2c ports found on board (or on monitors connected to the board) to be exposed.

Not tested with `nouveau` but this should also expose the i2c ports found on the board.

This is what `lspci -v` gives me:

```
07:00.3 Serial bus controller [0c80]: NVIDIA Corporation TU104 USB Type-C UCSI Controller (rev a1)
        Subsystem: ASUSTeK Computer Inc. TU104 USB Type-C UCSI Controller
        Flags: bus master, fast devsel, latency 0, IRQ 5
        Memory at f6084000 (32-bit, non-prefetchable) [size=4K]
        Capabilities: [68] MSI: Enable- Count=1/1 Maskable- 64bit+
        Capabilities: [78] Express Endpoint, MSI 00
        Capabilities: [b4] Power Management version 3
        Capabilities: [100] Advanced Error Reporting
```

## Easy way, with just i2c-dev 
```
apt install i2c-tools

modprobe i2c-dev
```

Now try to figure out which i2c device to use. We're looking for address 0x2a (which is from an ASUS ROG Strix 2070S):

```
# for bus in $(i2cdetect -l | grep NVIDIA | cut -d$'\t' -f 1 | cut -d "-" -f 2); do echo "i2c bus: $bus"; i2cdetect -y $bus| grep -B4 " 2a "; done
i2c bus: 3
i2c bus: 6
i2c bus: 4
i2c bus: 2
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- 08 -- -- -- -- -- -- -- 
10: -- -- -- -- -- 15 -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- 27 -- -- 2a -- -- -- -- -- 
i2c bus: 7
i2c bus: 5
```
Set fan at full speed:
```
i2cset 2 0x2a 0x41 0xFF
```
Change 0xFF for a lower value for lower speeds.


## Hard (but cleaner) way with kernel module

WiP


## Notes

```
# i2cdump 2 0x2a
No size specified (using byte-data access)
WARNING! This program can confuse your I2C bus, cause data loss and worse!
I will probe file /dev/i2c-2, address 0x2a, mode byte
Continue? [Y/n] 
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 00 00 00 00 00 00 00 01 00 01 00 83 00 00 00 00    .......?.?.?....
10: 00 00 00 00 00 35 00 00 00 00 00 00 00 00 08 01    .....5........??
20: 15 89 00 00 00 80 ff 32 3c 50 3c 46 50 03 00 ff    ??...?.2<P<FP?..
30: 00 00 02 00 00 00 00 00 00 00 00 00 00 cc 02 00    ..?..........??.
40: 02 8d 00 00 2f 01 00 00 30 01 00 00 00 00 00 00    ??../?..0?......
50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
```

same with 0x68


