# Hardware & system Requirements

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

# Install

## Easy way, userland with provided systemd unit

First, install requirements:
```
apt install i2c-tools nvidia-smi
modprobe i2c-dev
```

Then go in `asus_fc2_userland`. [Check out](#Detect-i2c-adapter) which i2c adapter to use, for example here: "2".

Set fan at full speed, to make sure everything is alright. (Change 0xFF for a lower value for lower speeds.)
```
i2cset 2 0x2a 0x41 0xFF
```

Edit the monitoring script `asus_fc2.sh` to your make sure everything is set up properly, and install the monitoring script & systemd service:
```
sudo make install
```

And that's it!

If you want to log every PWM value being set, edit `/etc/systemd/system/asus_fc2.service` and change:
```
ExecStart=/bin/bash /usr/local/bin/asus_fc2.sh -l
```
to
```
ExecStart=/bin/bash /usr/local/bin/asus_fc2.sh
```

## Hard way with kernel module

[Figure out](#Detect-i2c-adapter) your i2c device.

```
cd asus_fc2
make
sudo make install

modprobe asus_fc2

I2C_DEVICE_ADDRESS="0x2a" # << might be different if using a different Asus board than mine
I2C_BUS="2" # << might be different on your machine. Use i2cdetect method above to find it

echo asus_fc2 "${I2C_DEVICE_ADDRESS}"  > /sys/bus/i2c/devices/i2c-${I2C_BUS}/new_device>
```

And now you should have a new `hwmon` device you can use.

```
# for hwmon in /sys/class/hwmon/hwmon*; do if [[ "asus_fc2" == $(cat "${hwmon}/name") ]]; then  echo $hwmon ; fi; done
/sys/class/hwmon/hwmon2
# echo 255 > /sys/class/hwmon/hwmon2/pwm1
```
And hear your fan SPIN !

Unfortunately, I couldn't find a way to report the GPU temperature by reading on this i2c device. I actually couldn't even
find a way to get this information from the kernel space. `nouveau` driver is doing some [weird voodoo shit](https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/nouveau/nvkm/subdev/therm/gp100.c#L31)
to access it and export on their own module.

This means unfortunately, we can't use [fancontrol](https://github.com/lm-sensors/lm-sensors/blob/master/prog/pwm/fancontrol) to manage the fan speed
to cool down our GPU....


### Extra hard way with autodetection

For this to work we need to path the `nvidia` driver (might not be needed with `nouveau`).

I have it installed with dkms.

Patch it with the following (see Notes below as to why).
```
# diff -u /tmp/nv-i2c.c  /usr/src/nvidia-current-440.82/nvidia/nv-i2c.c
--- /tmp/nv-i2c.c	2020-04-15 23:59:55.488972454 +0200
+++ /usr/src/nvidia-current-440.82/nvidia/nv-i2c.c	2020-04-15 23:08:36.386176825 +0200
@@ -196,6 +196,7 @@

 struct i2c_adapter nv_i2c_adapter_prototype = {
     .owner             = THIS_MODULE,
+    .class	            = I2C_CLASS_HWMON,
     .algo              = &nv_i2c_algo,
     .algo_data         = NULL,
 };
```

Now rebuild your driver, and probably also reboot

```
dkms remove -m nvidia-current -v 440.82 -k all
dkms build -m nvidia-current -v 440.82 -k all
dkms install nvidia
reboot
```

And now that your `nvidia` i2c adapter is not a little bitch anymore, you can just do:

```
cd asus_fc2_module
make
sudo make install

modprobe asus_fc2

for hwmon in /sys/class/hwmon/hwmon*; do if [[ "asus_fc2" == $(cat "${hwmon}/name") ]]; then  echo $hwmon ; fi; done

echo 255 > /sys/class/hwmon/hwmon2/pwm1
```

You'll probably have to re-do this everytime the nvidia drivers get updated.


## Notes

I couldn't get autodetection to work, after implementing the i2c_driver->detect() function, it would actually not be called.

The [documentation](https://www.kernel.org/doc/html/latest/i2c/instantiating-devices.html) is not super clear as to what is going on:
```
Only buses which are likely to have a supported device and agree to be probed, will be probed. For example this avoids probing for hardware monitoring chips on a TV adapter.
```

So how can an i2c adapter decide not to be probed?

Detection should happen in [function i2c_detect_address in drivers/i2c/i2c-core-base.c](https://github.com/torvalds/linux/blob/v5.5/drivers/i2c/i2c-core-base.c#L2145), which is it self called by [i2c_detect()](https://github.com/torvalds/linux/blob/v5.5/drivers/i2c/i2c-core-base.c#L2215).

There are a bunch of `dev_dbg()` calls that might help us make sure we hit these functions. To enable their output to `dmesg` we need to enable [Dynamic debug](https://www.kernel.org/doc/html/v4.19/admin-guide/dynamic-debug-howto.html).


```
# enable all messages from file i2c-core.c

echo 'file drivers/i2c/i2c-core.c +p' > /sys/kernel/debug/dynamic_debug/control
```

With `asus_fc2.ko` built & installed with:
```
make && sudo make install
```
we see the following messages
```
Apr 15 00:02:55 pc kernel: [ 4372.671967] i2c-core: driver [asus_fc2] registered
Apr 15 00:02:55 pc kernel: [ 4372.671972] i2c i2c-0: found normal entry for adapter 0, addr 0x2a
Apr 15 00:02:55 pc kernel: [ 4372.672522] i2c i2c-1: found normal entry for adapter 1, addr 0x2a
```
But what about the other i2c busses? It turns out the nvidia-backed i2c adapter we want to talk to has id 2.

It looks like what prevents us from reaching the proper detection line is the following [code](https://github.com/torvalds/linux/blob/v5.5/drivers/i2c/i2c-core-base.c#L2201):
```
    /* Stop here if the classes do not match */
    if (!(adapter->class & driver->class))
        return 0;
```

While I suppose `driver->class` here is the one define in the `i2c_driver` struct (`.class = I2C_CLASS_HWMON`), but I couldn't find how to display the reported adapter class. Nothing shows up in relevant places in `/sys`.

I basically want MORE `dev_dbg` and see what's in the `i2c_adapter` struct in `i2c_detect()`. We have multiple solutions

 0. Add the calls to `dev_dbg()` in `i2c-core.c`, recompile the kernel, reboot.
 0. Use one of the recent live kernel patching methods like [kpatch](https://github.com/dynup/kpatch) to change the code.
 0. Use [bpftrace](https://github.com/iovisor/bpftrace) to hook that method and display its arguments.

Since I hate anything new, and change is terrible, I first tried solution 1. While the thing was compiling (it still takes, like 3 mins on my new machine! :3) I also looked at solution 2.
I couldn't make it work, and while complaining about how everything was terrible on IRC someone told me about eBPF and `bpftrace`.

It's actually quite simple to use once you figure out what you need to do.

First figure out if your function is traceable:

```
# bpftrace -l | grep i2c_detect
kprobe:i2c_detect
```

And write the small program that will take the first arg of `i2c_detect` and log the value of `class` for it:

```
$ cat i2c.bt
#include <linux/i2c.h>
#include <linux/i2c-smbus.h>

kprobe:i2c_detect
{
    $adapter = (struct i2c_adapter *)arg0;
    $driver = (struct i2c_driver *)arg1;
    printf("I2C adapter: %s \n", $adapter->name);
    printf("adapter class: %d, driver class: %d\n", $adapter->class, $driver->class);
}
```

Run `bpftrace`:
```
# bpftrace i2c.bt
Attaching 1 probe...
I2C adapter: SMBus PIIX4 adapter port 0 at 0b00
adapter class: 129, driver class: 1
I2C adapter: SMBus PIIX4 adapter port 2 at 0b00
adapter class: 129, driver class: 1
I2C adapter: NVIDIA i2c adapter 1 at 7:00.0
adapter class: 0, driver class: 1
I2C adapter: NVIDIA i2c adapter 3 at 7:00.0
adapter class: 0, driver class: 1
I2C adapter: NVIDIA i2c adapter 5 at 7:00.0
adapter class: 0, driver class: 1
I2C adapter: NVIDIA i2c adapter 6 at 7:00.0
adapter class: 0, driver class: 1
I2C adapter: NVIDIA i2c adapter 7 at 7:00.0
adapter class: 0, driver class: 1
I2C adapter: NVIDIA i2c adapter 8 at 7:00.0
adapter class: 0, driver class: 1
```

Ah ha ! So our nvidia i2c adapter is not declaring anyclass, which makes the test fail in `i2c_detect`, and the adapter to be ignored.

And indeed, in `/usr/src/nvidia-current-440.82/nvidia/nv-i2c.c` (installed via `nvidia-kernel-dkms` package), their `i2c_adapter` struct is instantiated without specifying the class:

```
struct i2c_adapter nv_i2c_adapter_prototype = {
    .owner             = THIS_MODULE,
    .algo              = &nv_i2c_algo,
    .algo_data         = NULL,
};
```

So you can patch it
```
struct i2c_adapter nv_i2c_adapter_prototype = {
    .owner             = THIS_MODULE,
	.class			   = I2C_CLASS_HWMON | I2C_CLASS_DDC, # Probably just need I2C_CLASS_HWMON
    .algo              = &nv_i2c_algo,
    .algo_data         = NULL,
};
```
Rebuild your module, and voila! Magic autodetection with a simple `modprobe`


## Misc

### Detect i2c adapter

Try to figure out which i2c device to use. We're looking for address 0x2a (which is from an ASUS ROG Strix 2070S):

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

This means the i2c bus is number 2.

### i2cdump

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
