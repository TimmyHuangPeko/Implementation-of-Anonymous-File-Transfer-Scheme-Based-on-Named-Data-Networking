#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct timeval timeval;

typedef struct DATA{
	char entire_name[32];
	char name[32];
	int segment;
	struct DATA* next;
	struct DATA* last;
}DATA;



DATA* data_head = NULL;
DATA* data_tail = NULL;



void data_enqueue(char* e_name, char* name, int segment)
{
	DATA* n = (DATA*)malloc(sizeof(DATA));
	
	
	strcpy(n->entire_name, e_name);
	
	strcpy(n->name, name);
	
	n->segment = segment;
	
	
	n->next = NULL;
	
	n->last = data_tail;
	
	if(data_tail == NULL){
		
		data_tail = n;
		data_head = n;
	}
	else{
		
		data_tail->next = n;
		data_tail = n;
	}
}



DATA* data_find(char* e_name)
{
	DATA* t = data_head;
	
	while(t != NULL){
		
		if(!strcmp(t->entire_name, e_name)) break;
		
		t = t->next;
	}
	
	return t;
}



void data_dequeue()
{	
	
	DATA* t = data_head;
	
	if(t != NULL){
		
		data_head = data_head->next;
		
		if(data_head != NULL){
			
			data_head->last = NULL;
		}
		else{
			
			data_tail = NULL;
		}
		
		free(t);
	}
}



void data_empty()
{
	DATA* p;
	
	while(data_head != NULL){
		
		p = data_head;
		data_head = data_head->next;
		free(p);
	}
	
	data_tail = NULL;
}



void data_print()
{
	DATA* t = data_head;
	int i = 1;
	
	printf("***********  DATA     list  ***********\n");
	
	while(t != NULL){
		
		printf("%2d.  %s\n", i, t->entire_name);
		
		t = t->next;
		
		i++;
	}
	
	printf("***************************************\n");
}




#endif
