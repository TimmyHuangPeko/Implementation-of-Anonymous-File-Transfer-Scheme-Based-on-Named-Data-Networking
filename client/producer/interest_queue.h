#ifndef INTEREST_QUEUE_H
#define INTEREST_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct timeval timeval;

typedef struct INTEREST{
	char identity[16];
	char name[32];
	FILE* file;
	int segment;
	timeval deadline;
	struct INTEREST* next;
	struct INTEREST* last;
}INTEREST;



INTEREST* interest_head = NULL;
INTEREST* interest_tail = NULL;



void interest_enqueue(char* identity, char* name, int lifetime)
{
	char request_name[50];
	
	sprintf(request_name, "./memory/%s", name);
	
	if(!access(request_name, F_OK)){
		
		strcat(request_name, "(1)");
		
		while(!access(request_name, F_OK)){
			
			request_name[strlen(request_name) - 2]++;
		}
	}
	
	
	INTEREST* n = (INTEREST*)malloc(sizeof(INTEREST));
	
	strcpy(n->identity, identity);
	
	strcpy(n->name, name);
	
	n->segment = 0;
	
	gettimeofday(&n->deadline, NULL);
	n->deadline.tv_sec += lifetime;
	
	n->file = fopen(request_name, "w");
	
	n->next = NULL;
	
	n->last = interest_tail;
	
	if(interest_tail == NULL){
		
		interest_tail = n;
		interest_head = n;
	}
	else{
		
		interest_tail->next = n;
		interest_tail = n;
	}
}



INTEREST* interest_find(char* name)
{
	INTEREST* t = interest_head;
	
	while(t){
		
		if(!strcmp(t->name, name)) break;
		
		t = t->next;
	}
	
	return t;
}



void interest_dequeue(INTEREST* t)
{	
	
	fclose(t->file); ///////////////////////////////////////////////////////////////
	
	if(t == NULL){
		
		printf("Fail to pop: can not find the corresponding INTEREST\n");
		return;
	}
	else if(t == interest_head && t == interest_tail){
		
		interest_head = NULL;
		interest_tail = NULL;
	}
	else if(t == interest_head){
		
		t->next->last = NULL;
		interest_head = t->next;
	}
	else if(t == interest_tail){
		
		t->last->next = NULL;
		interest_tail = t->last;
	}
	else{
		
		t->last->next = t->next;
		t->next->last = t->last;
	}
	
	free(t);
}



void interest_empty()
{
	INTEREST* p;
	
	while(interest_head != NULL){
		
		fclose(interest_head->file);
		
		p = interest_head;
		interest_head = interest_head->next;
		free(p);
	}
	
	interest_tail = NULL;
}



void interest_print()
{
	INTEREST* t = interest_head;
	int i = 1;
	
	printf("***********  request  list  ***********\n");
	
	while(t != NULL){
		
		printf("%2d.  %s, %s, handling segment: %d\n", i, t->name, t->identity, t->segment);
		
		t = t->next;
		
		i++;
	}
	
	printf("***************************************\n");
}



void interest_lifetime_check()
{
	timeval now;
	INTEREST* t = interest_head;
	INTEREST* p = NULL;
	
	gettimeofday(&now, NULL);
	
	while(t != NULL){
		
		if(now.tv_sec > t->deadline.tv_sec || (now.tv_sec == t->deadline.tv_sec && now.tv_usec >= t->deadline.tv_usec)){
			
			p = t;
			t = t->next;
			
			interest_dequeue(p);
			
			continue;
		}
		
		t = t->next;
	}
}



#endif
