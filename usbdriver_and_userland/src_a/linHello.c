#include <linux/errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

#define __NR_HELLO 316

long lin_hello(void){
	return (long) syscall(__NR_HELLO);
}

int main(){
	return lin_hello();
}
