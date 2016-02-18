#include <linux/module.h> 
#include <asm-generic/errno.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0
#define BUFFER_LENGTH       PAGE_SIZE/2


struct tty_driver* kbd_driver= NULL;
static struct proc_dir_entry *proc_entry;
static char *info;  // buffer para almacenar la entrada de texto



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


static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off){

	return 0; 
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
	unsigned int mask;
	
	kbd_driver= get_kbd_driver_handler();
	
	if (copy_from_user(&info[0], buf, len))
		return -EFAULT;

	sscanf(info,"0x%x", &mask);
	
	
	set_leds(kbd_driver,mask); 
	

	
	
	return len;
}




static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,    
};


static int __init modleds_init(void)
{	
   kbd_driver= get_kbd_driver_handler();
   
   
   
   info = (char *)vmalloc( BUFFER_LENGTH ); //reservamos memoria para el buffer de entrada

	if (!info) 
		return ENOMEM;
	else {

		memset( info, 0, BUFFER_LENGTH );
		proc_entry = proc_create( "modleds", 0666, NULL, &proc_entry_fops);
    
		if (proc_entry == NULL) {
			vfree(info);
			printk(KERN_INFO "no se puede insertar el modulo \n");
			return ENOMEM;
		} 
		else 
      printk(KERN_INFO "modlist: Module loaded\n");
  }
   
    unsigned int mask = 0x2;
   
   
    unsigned int status = 0, i = 0;
    for (i = 0 ; i < 3 ; i++){
    
		 if (mask & (0x1 << i)){
	   
		   if (i == 1) status |= 0x4;
		   else if (i == 2) status |= 0x2;
				  
		}
    
    }
   
   
   
   
   
   set_leds(kbd_driver, status); 
   return 0;
}

static void __exit modleds_exit(void){
    set_leds(kbd_driver,ALL_LEDS_OFF); 
    remove_proc_entry("modleds", NULL);
  vfree(info);
  printk(KERN_INFO "modlist: Module unloaded.\n");
}

module_init(modleds_init);
module_exit(modleds_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modleds");
