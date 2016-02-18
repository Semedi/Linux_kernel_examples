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
	int i, salir = 0;;
	sscanf(argv[1], "0x%x", &mask);
	printf("%x\n", mask);
	
	switch(mask){
		case 4:
			for(i = 1; i <= 4 ; i*=2){
				ledctl(i);
				usleep(500000);
			}
		break;
		
		case 2:
			
			for(i = 4; i >= 1 && salir == 0 ; i/=2){
				if(i == 1)
					salir = 1;
				ledctl(i);
				usleep(500000);
			}
		break;
	}
	//return ledctl(mask);
	return 0;
}
