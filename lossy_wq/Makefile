obj-m   += module.o

module-y := main.o

KERNELDIR ?= /usr/src/kernels/`uname -r`/
ccflags-y := -I$(KERNELDIR)/include/
PWD := $(shell pwd)
all:
	make -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	make -C /lib/modules/`uname -r`/build/ M=`pwd` clean

install:
	insmod module.ko
	sleep 1
	tail -n 20 /var/log/messages
