obj-m += character_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	-sudo rmmod character_device
	sudo dmesg -C
	sudo insmod character_device.ko
	dmesg
