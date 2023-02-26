#ifndef ID_QUEUE_H
#define ID_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "RSA.h"

typedef struct KEY{
	char identity[16];
	RSA keys;
	struct KEY* next;
	struct KEY* last;
}KEY;



KEY* key_head = NULL;
KEY* key_tail = NULL;



void key_enqueue(char* identity, RSA keys)
{
	KEY* n = (KEY*)malloc(sizeof(KEY));
	
	
	strcpy(n->identity, identity);
	
	memcpy(n->keys.e.t, keys.e.t, sizeof(LINT));
	
	memcpy(n->keys.d.t, keys.d.t, sizeof(LINT));
	
	memcpy(n->keys.n.t, keys.n.t, sizeof(LINT));
	
	
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



KEY* key_find(char* identity)
{
	KEY* t = key_head;
	
	while(t != NULL){
		
		if(!strcmp(t->identity, identity)) break;
		
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
		
		printf("%2d.  %s\n", i, t->identity);
		
		t = t->next;
		
		i++;
	}
	
	printf("***************************************\n");
}




#endif
