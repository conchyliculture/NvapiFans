modname := asus_fc2
obj-m := $(modname).o

KVERSION := $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build
PWD := "$$(pwd)"

ifdef DEBUG
CFLAGS_$(obj-m) := -DDEBUG
endif

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) O=$(PWD) -C $(KDIR) M=$(PWD) clean

load:
	rmmod $(modname)
	insmod $(modname).ko

install:
	install -m 0755 -o root -g root $(modname).ko /lib/modules/$(KVERSION)/kernel/drivers/hwmon/
	depmod -a
	echo ${modname} > /etc/modules-load.d/${modname}.conf

uninstall:
	rm /etc/modules-load.d/${modname}.conf
	rm /lib/modules/$(KVERSION)/kernel/drivers/hwmon/$(modname).ko
	depmod -a
