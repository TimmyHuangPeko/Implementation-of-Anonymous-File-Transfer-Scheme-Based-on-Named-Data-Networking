#include "ndn.h"


#define InterestTime 10
#define DataTime 1000

typedef struct Node{
	struct InterestPacket Packet;
	int num;
	struct Node* next;
}Node;

void insert(Node* head, struct InterestPacket Packet, int num){

	/*===================================
	
		Insert Packet in Stack
	
	===================================*/
	
	Node* New = (Node*) malloc(sizeof(Node));
	New->Packet = Packet;
	New->num = num;
	New->next = head->next;
	head->next = New;
}

Node* search(Node* head, int type, char* name){

	/*===================================
	
		Search Packet from Stack
	
	===================================*/
	
	Node* c = head->next;
	while(c != 0){
		if(c->Packet.head.type == type && !strcmp(name, c->Packet.head.name)){
			return c;
		}
		c = c->next;
	}
	
	return NULL;
		
}

void pop(Node* head, struct InterestPacket Packet){

	/*===================================
	
		Pop Packet from Stack
	
	===================================*/
	
	Node* c = head->next;
	
	while(c->next != 0){
		if( !strcmp(c->next->Packet.head.name,Packet.head.name) )
			break;
		c = c->next;
	}
	
	if( !strcmp(c->next->Packet.head.name,Packet.head.name) ){
		Node* n = c->next;
		c->next = n->next;
		free(n);
		return;
	}
	
}

void LRU_pop(Node* head){
	
	Node* c = head;
	
	while(c -> next != 0){
		if(c->next->next == 0)
			break;
		
		c = c->next;
	}
	
	if( c->next == 0 ){
		head -> next = 0;
		free(c);
	}
	else{
		Node* d = c->next;
		c->next = 0;
		free(d);
	}
}

void Print(Node* stack, Node* CS){

	/*=========================================
	
		Print All Packet from Stack
	
	=========================================*/

	Node* c = stack->next;
	
	printf("PIT : \n");
	while(c != 0){
		printf("\tFD:%d Name:%s Type:%d Nonce:%d Len:%d\n", c->num, c->Packet.head.name, c->Packet.head.type, c->Packet.head.nonce, c->Packet.head.len); 
		c = c->next;
	}
	
	c = CS->next;
	
	printf("\nContent Store : \n");
	while(c != 0){
		printf("\tFD:%d Name:%s Type:%d Nonce:%d Len:%d\n", c->num, c->Packet.head.name, c->Packet.head.type, c->Packet.head.nonce, c->Packet.head.len); 
		c = c->next;
	}
	
}

bool Nonce(Node* head, struct InterestPacket Packet){

	/*==========================================
	
		Search Same Packet from Stack
	
	==========================================*/

	Node* c = head->next;
	
	while(c != 0){
		if(!strcmp(Packet.head.name, c->Packet.head.name) && Packet.head.type == c->Packet.head.type && c->Packet.head.nonce == Packet.head.nonce)
			return true;
		c = c->next;
	}
	return false;
}

void TimePop(Node* head, struct timeval time){

	/*===================================
	
		Pop All Overtime Packet
	
	===================================*/

	Node* c = head;
	
	while(c->next != 0){
		
		if( (c->next->Packet.head.type == 1 && time.tv_sec - c->next->Packet.head.time.tv_sec >= InterestTime) 
		|| (c->next->Packet.head.type == 2 && time.tv_sec - c->next->Packet.head.time.tv_sec >= DataTime) ){
			
			Node* n = c->next;
			c->next = n->next;
			
			free(n);
			
		}
		else{
			c = c->next;
		}
		
		
	}
	
	if( c != NULL && (c->Packet.head.type == 1 && time.tv_sec - c->Packet.head.time.tv_sec >= InterestTime) 
	|| (c->Packet.head.type == 2 && time.tv_sec - c->Packet.head.time.tv_sec >= DataTime) ){
		
		Node* n = c->next;
		c->next = n->next;
			
		free(n);
		
	}
	
}

bool DataGive(Node* head, int fd, char* name, int ID){

	/*===================================
	
		Search Packet from Stack
	
	===================================*/

	Node* c = head;
	bool b = 0;
	
	while(c != 0 && c->next != 0){
		if( (c->next->Packet.head.type == 2 && !strcmp(c->next->Packet.head.name, name)) ){
			b = true;
			
			write(fd, &c->next->Packet, sizeof(Head) + sizeof(LINT) + c->next->Packet.head.len);
			usleep(1);
			printf("SEND Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", c->next->num, c->next->Packet.head.name, c->next->Packet.head.type, c->next->Packet.head.nonce, c->next->Packet.head.len);
	
			
			if(c == head){
				c = c->next;
				continue;
			}
			
			Node* n = c->next;
			c->next = c->next->next;
			n->next = head->next;
			head->next = n;
			
			
			
			continue;
		}
		c = c->next;
	}
	
	return b;
}
