obj-m = mfifo.o #mfifo.c no ha de existir

mfifo-objs = multififo.o cbuffer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
