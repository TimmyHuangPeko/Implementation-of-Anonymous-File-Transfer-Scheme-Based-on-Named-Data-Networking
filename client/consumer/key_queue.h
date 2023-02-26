#ifndef KEY_QUEUE_H
#define KEY_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


typedef struct KEY{
	char name[16];
	char public_key[1024];
	char n_key[1024];
	struct KEY* next;
	struct KEY* last;
}KEY;



KEY* key_head = NULL;
KEY* key_tail = NULL;



void key_enqueue(char* name, char* public_key, char* n_key)
{
	KEY* n = (KEY*)malloc(sizeof(KEY));
	
	
	strcpy(n->name, name);
	
	memcpy(n->public_key, public_key, sizeof(LINT));
	
	memcpy(n->n_key, n_key, sizeof(LINT));
	
	
	n->next = NULL;
	
	n->last = key_tail;
	
	if(key_tail == NULL){
		
		key_tail = n;
		key_head = n;
	}
	else{
		
		key_tail->next = n;
		key_tail = n;
	}
}



KEY* key_find(char* name)
{
	KEY* t = key_head;
	
	while(t != NULL){
		
		if(!strcmp(t->name, name)) break;
		
		t = t->next;
	}
	
	return t;
}



void key_dequeue()
{	
	
	KEY* t = key_head;
	
	if(t != NULL){
		
		key_head = key_head->next;
		
		if(key_head != NULL){
			
			key_head->last = NULL;
		}
		else{
			
			key_tail = NULL;
		}
		
		free(t);
	}
}



void key_empty()
{
	KEY* p;
	
	while(key_head != NULL){
		
		p = key_head;
		key_head = key_head->next;
		free(p);
	}
	
	key_tail = NULL;
}



void key_print()
{
	KEY* t = key_head;
	int i = 1;
	
	printf("***********  KEY      list  ***********\n");
	
	while(t != NULL){
		
		printf("%2d.  %s\n", i, t->name);
		
		t = t->next;
		
		i++;
	}
	
	printf("***************************************\n");
}




#endif
