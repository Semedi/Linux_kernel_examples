#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include <linux/list.h>

#include "cbuffer.h"


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sergio Semedi Lin final Module - FDI-UCM");

#define BUFFER_LENGTH       PAGE_SIZE/8
#define MAX_CBUFFER_LEN 100


struct list_head _files; /* Lista enlazada */

typedef struct fifo_resources {
	cbuffer_t *cbuffer;	
	struct semaphore sem_prod; /* cola de espera para productor(es) */
	struct semaphore sem_cons; /* cola de espera para consumidor(es) */
	struct semaphore mtx; /* para garantizar Exclusión Mutua */
	int nr_prod_waiting; /* Número de procesos productores esperando */
	int nr_cons_waiting; /* Número de procesos consumidores esperando */
	int prod_count;
	int cons_count;
	
};


/* Nodos de la lista */
typedef struct list_item_t {
	char *name;
	struct fifo_resources fifo;
	struct list_head links;
};

struct proc_dir_entry *multififo_dir=NULL;
static struct proc_dir_entry *control_entry;







DEFINE_SPINLOCK(sp); //Spin-lock

static ssize_t multififo_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
	
	
	struct fifo_resources *res =(struct fifo_resources*)PDE_DATA(filp->f_inode);

	
	char kbuffer[MAX_CBUFFER_LEN];
	if (len> MAX_CBUFFER_LEN) {
		return -EFAULT;
	}
	if (down_interruptible(&(res->mtx))){
		return -EINTR;
	}
	while (size_cbuffer_t(res->cbuffer)<len && res->prod_count>0){
		res->nr_cons_waiting +=1;
		up(&(res->mtx));
		if (down_interruptible(&(res->sem_cons))){
			return -EINTR;	
		}
	}
	
	if(res->prod_count == 0 && size_cbuffer_t(res->cbuffer) == 0){
		up(&(res->mtx));
		return 0;
	}
	
	remove_items_cbuffer_t(res->cbuffer,kbuffer,len);
	
	if(res->nr_prod_waiting > 0){
		up(&(res->sem_prod));
		res->nr_prod_waiting -=1;
	}
	up(&(res->mtx));
	
	if (copy_to_user(buf,kbuffer,len)) {
		return -EFAULT;
	}
	return len;	
}

static ssize_t multififo_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){

	char kbuffer[MAX_CBUFFER_LEN];
	
	struct fifo_resources *res =(struct fifo_resources*)PDE_DATA(filp->f_inode);
	
	if(len > MAX_CBUFFER_LEN ){ 
		return -EFAULT;
	}
	if (copy_from_user(kbuffer,buf,len)) {
		return -EFAULT;
	}
	if (down_interruptible(&(res->mtx))){
		return -EINTR;
	}
	/* Esperar hasta que haya hueco para insertar (debe haber consumidores) */
	while (nr_gaps_cbuffer_t(res->cbuffer)<len && res->cons_count>0){
		res->nr_prod_waiting +=1;
		up(&(res->mtx));
		if (down_interruptible(&(res->sem_prod))){
			return -EINTR;
		}	
	}
	
	/* Detectar fin de comunicación por error (consumidor cierra FIFO antes) */
	if (res->cons_count==0) {
		up(&(res->mtx)); 
		return -EPIPE;
	}
	
	insert_items_cbuffer_t(res->cbuffer,kbuffer,len);
	/* Despertar a posible consumidor bloqueado */
	if(res->nr_cons_waiting > 0){
		up(&(res->sem_cons));
		res->nr_cons_waiting -=1;
	}
	up(&(res->mtx));
	return len;
}

/* Se invoca al hacer open() de entrada /proc */
static int multififo_open(struct inode *inode, struct file *file)
{
	
	struct fifo_resources* res =(struct fifo_resources*)PDE_DATA(file->f_inode);
	
	if (down_interruptible(&(res->mtx))){
		return -EINTR;
	}
	if(file->f_mode & FMODE_READ){
		res->cons_count +=1;
		//cond_signal(prod)
		up(&(res->sem_prod));
		res->nr_prod_waiting -=1;
		while(res->prod_count == 0){
			//cond_wait(cons,mtx);
			res->nr_cons_waiting +=1;
			up(&(res->mtx));
			if (down_interruptible(&(res->sem_cons))){
				return -EINTR;
			}
		}
		
	}else{
		res->prod_count +=1;
		//cond_signal(cons)
		up(&(res->sem_cons));
		res->nr_cons_waiting -=1;
		while(res->cons_count == 0){
			//cond_wait(cons,mtx);
			res->nr_prod_waiting +=1;
			up(&(res->mtx));
			if (down_interruptible(&(res->sem_prod))){
				return -EINTR;
			}	
		}
		
	}
	up(&(res->mtx));
	return 0;
}


/* Se invoca al hacer close() de entrada /proc */
static int multififo_release(struct inode *inode, struct file *file){
	
	struct fifo_resources *res =(struct fifo_resources*)PDE_DATA(file->f_inode);
	
	if (down_interruptible(&(res->mtx))){
		return -EINTR;
	}
	if(file->f_mode & FMODE_READ){
		res->cons_count -=1;
		//cond_signal(prod)
		if(res->nr_prod_waiting > 0){
			up(&(res->sem_prod));
			res->nr_prod_waiting -=1;
		}
		
	}else{
		res->prod_count -=1;
		//cond_signal(cons)
		if(res->nr_cons_waiting > 0){
			up(&(res->sem_cons));
			res->nr_cons_waiting -=1;
		}
		
	}
	if(res->prod_count == 0 && res->cons_count == 0)
		clear_cbuffer_t(res->cbuffer);
	up(&(res->mtx));
	return 0;
}




static const struct file_operations proc_entry_fifo_list = {
    .read = multififo_read,
    .write = multififo_write,    
    .open 	 = multififo_open,
	.release = multififo_release,    
};


static ssize_t control_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
  
	struct list_item_t* tmp=NULL;
	struct list_item_t* node=NULL;
	struct list_head *pos, *q;
	
	int flag = 0;
	
    char kbuf[BUFFER_LENGTH];
    char name[BUFFER_LENGTH];
    
    if (copy_from_user(kbuf, buf, len))
		return -EFAULT;
	
	if (sscanf(kbuf, "create %s", name)){
		node = (struct list_item_t *)vmalloc(sizeof(struct list_item_t));
		
		/* Resources 4 every fifo*/
		node->fifo.cbuffer = create_cbuffer_t(MAX_CBUFFER_LEN);	
		node->fifo.nr_cons_waiting = 0;
		node->fifo.nr_prod_waiting = 0;
		node->fifo.prod_count = 0;
		node->fifo.cons_count = 0;   
		sema_init(&(node->fifo.sem_cons),0); 
		sema_init(&(node->fifo.sem_prod),0); 
		sema_init(&(node->fifo.mtx),1);
		
		
		control_entry = proc_create_data(name, 0666, multififo_dir, &proc_entry_fifo_list, &node->fifo);
		node->name = vmalloc(strlen(name)+1);
		strcpy(node->name, name);
		INIT_LIST_HEAD(&node->links);
		
		
		spin_lock(&sp);
		list_add_tail(&node->links, &_files);
		spin_unlock(&sp);
		
	}
	if (sscanf(kbuf, "delete %s", name)){
		
		
		
		spin_lock(&sp);
		list_for_each_safe(pos, q, &_files){
			 tmp= list_entry(pos, struct list_item_t, links);
			 if (!strcmp(name, tmp->name)){
				 destroy_cbuffer_t(tmp->fifo.cbuffer);
				 vfree(tmp->name);
				 list_del(pos);
				 vfree(tmp);
				 flag=1;
		 }
		}
		spin_unlock(&sp);
		
		if (flag)
			remove_proc_entry(name, multififo_dir);
		
		
	}

    return len;
}

static ssize_t control_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{

    int nr_bytes=0;

    return nr_bytes;
}



static const struct file_operations control_entry_fops = {
    .read = control_read,
    .write = control_write,
};


int init_multififo_module( void )
{
	struct list_item_t *node  = NULL;
	
	INIT_LIST_HEAD(&_files); //inicializamos la lista
	
	//Inicializamos semaforos de mutex y de colas de espera
	
	
	
	
	multififo_dir=proc_mkdir("multififo",NULL);
	
	if (!multififo_dir)
		return -ENOMEM;
		
	  /* Create proc entry /proc/test/clipboard */
    control_entry = proc_create( "control", 0666, multififo_dir, &control_entry_fops);
    
    
    if (control_entry == NULL ) 
        return -ENOMEM;
    
   
	
	node = (struct list_item_t *)vmalloc(sizeof(struct list_item_t));
	
	/* Resources 4 every fifo*/
    node->fifo.cbuffer = create_cbuffer_t(MAX_CBUFFER_LEN);	
    node->fifo.nr_cons_waiting = 0;
    node->fifo.nr_prod_waiting = 0;
    node->fifo.prod_count = 0;
    node->fifo.cons_count = 0;   
    sema_init(&(node->fifo.sem_cons),0); 
    sema_init(&(node->fifo.sem_prod),0); 
    sema_init(&(node->fifo.mtx),1);
    
    
	control_entry = proc_create_data( "default", 0666, multififo_dir, &proc_entry_fifo_list, &node->fifo);
	node->name = vmalloc(strlen("default")+1);
	strcpy(node->name, "default");
	INIT_LIST_HEAD(&node->links);
	
	spin_lock(&sp);
	list_add_tail(&node->links, &_files);
	spin_unlock(&sp);
	
	if (control_entry == NULL ) {
        remove_proc_entry("default", NULL);
        vfree(node);
        return -ENOMEM;
    }
	
	
	
    

	

    printk(KERN_INFO "Clipboard: Multififo loaded\n");

    return 0;
}




void exit_multififo_module( void )
{
	
	struct list_item_t* tmp=NULL;
	struct list_head *pos, *q;

	list_for_each_safe(pos, q, &_files){
		 tmp= list_entry(pos, struct list_item_t, links);
		 destroy_cbuffer_t(tmp->fifo.cbuffer);
		 remove_proc_entry(tmp->name, multififo_dir);
		 vfree(tmp->name);
		 list_del(pos);
		 vfree(tmp);
		 
		 
		 
	}
	
	
	remove_proc_entry("control", multififo_dir);
	remove_proc_entry("multififo", NULL);
   
    printk(KERN_INFO "multififo: Module removed.\n");
}



module_init( init_multififo_module );
module_exit( exit_multififo_module );
