#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/module.h> 
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0


struct tty_driver* kbd_driver= NULL;


/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

SYSCALL_DEFINE1(ledctl, unsigned int, leds){
	struct tty_driver* handler = get_kbd_driver_handler();
    
     unsigned int status = 0, i = 0;
    for (i = 0 ; i < 3 ; i++){
    
     if (leds & (0x1 << i)){
       if (i == 0)
         status |= 0x1;
       else if (i == 1)
         status |= 0x4;
       else if (i == 2)
         status |= 0x2;
              
     }
    
    }
    
	set_leds(handler, status);
	return 0;
}

