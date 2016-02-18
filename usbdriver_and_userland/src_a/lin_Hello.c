#include <linux/syscalls.h>
#include <linux/kernel.h>

SYSCALL_DEFINE0(lin_hello){
	printk(KERN_DEBUG "Hello LIN'Teacher\n");
	return 0;
}
