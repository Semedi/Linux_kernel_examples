#include "cbuffer.h"
#ifdef __KERNEL__
#include <linux/vmalloc.h> /* vmalloc()/vfree()*/
#include <asm/string.h> /* memcpy() */
#else
#include <stdlib.h>
#include <string.h>
#endif

#ifndef NULL
#define NULL 0
#endif

/* Create cbuffer */
cbuffer_t* create_cbuffer_t (unsigned int max_size)
{
#ifdef __KERNEL__ 
	cbuffer_t *cbuffer= (cbuffer_t *)vmalloc(sizeof(cbuffer_t));
#else
	cbuffer_t *cbuffer= (cbuffer_t *)malloc(sizeof(cbuffer_t));
#endif
	if (cbuffer == NULL)
	{
	    return NULL;
	}
	cbuffer->size=0;
	cbuffer->head=0;
	cbuffer->max_size=max_size;

	/* Stores bytes */
#ifdef __KERNEL__ 
	cbuffer->data=vmalloc(max_size);
#else
	cbuffer->data=malloc(max_size);
#endif
	if ( cbuffer->data == NULL)
	{
#ifdef __KERNEL__ 
		vfree(cbuffer->data);
#else
		free(cbuffer->data);
#endif
		return NULL;
	}
	return cbuffer;
}

/* Release memory from circular buffer  */
void destroy_cbuffer_t ( cbuffer_t* cbuffer )
{
    cbuffer->size=0;
    cbuffer->head=0;
    cbuffer->max_size=0;
#ifdef __KERNEL__ 
    vfree(cbuffer->data);
    vfree(cbuffer);
#else
    free(cbuffer->data);
    free(cbuffer);
#endif
}

/* Returns the number of elements in the buffer */
int size_cbuffer_t ( cbuffer_t* cbuffer )
{
	return cbuffer->size ;
}

int nr_gaps_cbuffer_t ( cbuffer_t* cbuffer )
{
	return cbuffer->max_size-cbuffer->size;
}

/* Return a non-zero value when buffer is full */
int is_full_cbuffer_t ( cbuffer_t* cbuffer )
{
	return ( cbuffer->size == cbuffer->max_size ) ;
}

/* Return a non-zero value when buffer is empty */
int is_empty_cbuffer_t ( cbuffer_t* cbuffer )
{
	return ( cbuffer->size == 0 ) ;
}


/* Inserts an item at the end of the buffer */
void insert_cbuffer_t ( cbuffer_t* cbuffer, char new_item )
{
	unsigned int pos=0;
	/* The buffer is full */
	if ( cbuffer->size == cbuffer->max_size )
	{
		/* Overwriting head position */
		cbuffer->data[cbuffer->head]=new_item;
		/* Now head position must be the next one*/
		if ( cbuffer->size !=0 )
			cbuffer->head= ( cbuffer->head+1 ) % cbuffer->max_size;
		/* Size remains constant*/
	}
	else
	{
		if ( cbuffer->max_size!=0 )
			pos= ( cbuffer->head+cbuffer->size ) % cbuffer->max_size;
		cbuffer->data[pos]=new_item;
		cbuffer->size++;
	}
}

/* Inserts nr_items into the buffer */
void insert_items_cbuffer_t ( cbuffer_t* cbuffer, const char* items, int nr_items)
{
	int nr_items_left=nr_items;
	int items_copied;
	int nr_gaps=cbuffer->max_size-cbuffer->size;
	int whead=(cbuffer->head+cbuffer->size)%cbuffer->max_size;
	
	/* Restriction: nr_items can't be greater than the max buffer size) */
	if (nr_items>cbuffer->max_size)
		return;
	
	/* Check if we can't store all items at the end of the buffer */
	if (whead+nr_items_left > cbuffer->max_size)
	{
		items_copied=cbuffer->max_size-whead;
		memcpy(&cbuffer->data[whead],items,items_copied);
		nr_items_left-=items_copied;
		items+=items_copied; //Move the pointer forward
		whead=0;		
	}
	
	/* If we still have to copy elements -> do it*/
	if (nr_items_left)
	{
		memcpy(&cbuffer->data[whead],items,nr_items_left);
		whead+=nr_items_left;
	}
	
	/* Update size and head */
	if (nr_gaps>=nr_items)
	{
		cbuffer->size+=nr_items;
	}else
	{
		cbuffer->size=cbuffer->max_size;
		/* head moves in the event we overwrite stuff */
		cbuffer->head=(cbuffer->head+(nr_items-nr_gaps))% cbuffer->max_size;
	}
}

/* Removes nr_items from the buffer and returns a copy of them */
void remove_items_cbuffer_t ( cbuffer_t* cbuffer, char* items, int nr_items)
{
	int nr_items_left=nr_items;
	int items_copied;
	
	/* Restriction: nr_items can't be greater than the buffer size (Ignore)) */
	if (nr_items>cbuffer->size)
		return;	
	
	/* Check if we can't store all items at the end of the buffer */
	if (cbuffer->head+nr_items_left > cbuffer->max_size)
	{
		items_copied=cbuffer->max_size-cbuffer->head;
		memcpy(items,&cbuffer->data[cbuffer->head],items_copied);
		nr_items_left-=items_copied;
		items+=items_copied; //Move the pointer forward
		cbuffer->head=0;		
	}
	
	
	/* If we still have to copy elements -> do it*/
	if (nr_items_left)
	{
		memcpy(items,&cbuffer->data[cbuffer->head],nr_items_left);
		cbuffer->head+=nr_items_left;
	}
	
	/* Update size */
	cbuffer->size-=nr_items;
}


/* Remove first element in the buffer */
char remove_cbuffer_t ( cbuffer_t* cbuffer)
{
	char ret='\0';
	
	if ( cbuffer->size !=0 )
	{
		ret=cbuffer->data[cbuffer->head];	
		cbuffer->head= ( cbuffer->head+1 ) % cbuffer->max_size;
		cbuffer->size--;
	}
	
	return ret;
}

/* Removes all items in the buffer */
void clear_cbuffer_t (cbuffer_t* cbuffer) { 
	cbuffer->size = 0; 
	cbuffer->head = 0;
}

/* Returns the first element in the buffer */
char* head_cbuffer_t ( cbuffer_t* cbuffer )
{
	if ( cbuffer->size !=0 )
		return &cbuffer->data[cbuffer->head];
	else{
		return NULL;
	}
}



