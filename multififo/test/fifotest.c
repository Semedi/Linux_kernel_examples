#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <err.h>
#include <errno.h>
#define MAX_MESSAGE_SIZE 32

char* nombre_programa=NULL;

struct fifo_message {
	unsigned int nr_bytes;
	char data[MAX_MESSAGE_SIZE];
};


static void fifo_send (const char* path_fifo) {
  struct fifo_message message;
  int fd_fifo=0;
  int bytes=0,wbytes=0;
  const int size=sizeof(struct fifo_message);

  fd_fifo=open(path_fifo,O_WRONLY);

  if (fd_fifo<0) {
	perror(path_fifo);
	exit(1);
  }
  
 /* Bucle de envío de datos a través del FIFO
    - Leer de la entrada estandar hasta fin de fichero
 */
  while((bytes=read(0,message.data,MAX_MESSAGE_SIZE))>0) {
	message.nr_bytes=bytes;
	wbytes=write(fd_fifo,&message,size);

	if (wbytes > 0 && wbytes!=size) {
		fprintf(stderr,"Can't write the whole register\n");
		exit(1);
  	}else if (wbytes < 0){
		perror("Error when writing to the FIFO\n");
		exit(1);
  	}		
  }
  
  if (bytes < 0) {
	fprintf(stderr,"Error when reading from stdin\n");
	exit(1);
  }
  
  close(fd_fifo);
}

static void fifo_receive (const char* path_fifo) {
  struct fifo_message message;
  int fd_fifo=0;
  int bytes=0,wbytes=0;
  const int size=sizeof(struct fifo_message);

  fd_fifo=open(path_fifo,O_RDONLY);

  if (fd_fifo<0) {
	perror(path_fifo);
	exit(1);
  }


  while((bytes=read(fd_fifo,&message,size))==size) {
	/* Write to stdout */
	wbytes=write(1,message.data,message.nr_bytes);
	
	if (wbytes!=message.nr_bytes) {
		fprintf(stderr,"Can't write data to stdout\n");
		exit(1);
  	}	
 }

  if (bytes > 0){
	fprintf(stderr,"Can't read the whole register\n");
	exit(1);
  }else if (bytes < 0) {
	fprintf(stderr,"Error when reading from the FIFO\n");
	exit(1);
  }
	
   close(fd_fifo);
}

static void
uso (int status)
{
  if (status != EXIT_SUCCESS)
    warnx("Pruebe `%s -h' para obtener mas informacion.\n", nombre_programa);
  else
    {
      printf ("Uso: %s -f <path_fifo> [OPCIONES]\n", nombre_programa);
fputs ("\
  -r,  el proceso actúa como receptor de los mensajes el FIFO\n\
  -s,  el proceso envía los mensajes leidos de la entrada estandar por el FIFO\n\
", stdout);
      fputs ("\
  -h,	Muestra este breve recordatorio de uso\n\
", stdout);
    }
    exit (status);
}

int
main (int argc, char **argv)
{
  int optc;
  char* path_fifo=NULL;
  int receive=0;
  nombre_programa = argv[0];

  while ((optc = getopt (argc, argv, "srhf:")) != -1)
    {
      switch (optc)
	{

	case 'h':
	  uso(EXIT_SUCCESS);
	  break;
	
	case 'r':
	  receive=1;
	  break;

	case 's':
	  receive=0;
	  break;	

	case 'f':
	  path_fifo=optarg;
	  break;	

	default:
	  uso (EXIT_FAILURE);
	}
    }

 if (!path_fifo)
	uso(EXIT_FAILURE);
 
  if (receive)
	fifo_receive(path_fifo);
  else
	fifo_send(path_fifo);

  exit (EXIT_SUCCESS);
}
