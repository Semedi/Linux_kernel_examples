#include <linux/errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char* argv[]){
	
	//entrada 
	printf("hola que tal estas \n");
	
	
	
	
	char color[6]="0xFFFFFF";
	
	int cnt = 0;
	char buffer[8];
	
	int dispositivo = open ("/dev/usb/blinkstick0", O_WRONLY);
	while(1){
		cnt= cnt%7;
		write(dispositivo, cnt+":0x001100", strlen("1:0x001100"));
		cnt+=1;
		usleep(100000);
		
	}
	
	close(dispositivo);

}
