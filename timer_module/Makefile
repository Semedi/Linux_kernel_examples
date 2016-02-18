obj-m = timermod.o #fifomod.c no ha de existir

timermod-objs = modtimer.o cbuffer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

