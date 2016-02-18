#include <linux/errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

#define __NR_LEDCTL 317

long ledctl(unsigned int mask){
	return (long) syscall(__NR_LEDCTL, mask);
}

int main(int argc, char* argv[]){
	unsigned int mask;
	sscanf(argv[1], "0x%x", &mask);
	printf("%x\n", mask);
	return ledctl(mask);
}
