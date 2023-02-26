#include "interest_queue.h"

int main(void)
{
	char cmd[50] = "";
	char action[10] = "";
	int t;
	
	while(1){
		
		fgets(cmd, sizeof(cmd), stdin);
		
		if(!strncmp(cmd, "enqueue", 7)){
			
			char name[32] = "";
			int nonce;
			int lifetime;
			
			t = sscanf(cmd, "%s %s %d %d\n", action, name, &nonce, &lifetime);
			printf("%s %d %d\n",name, nonce, lifetime);
			
			interest_enqueue(name, nonce, lifetime);
		}
		else if(!strncmp(cmd, "dequeue", 7)){
			
			char name[32] = "";
			int nonce;
			
			t = sscanf(cmd, "%s %s %d\n", action, name, &nonce);
			
			interest_dequeue(interest_find(name, nonce));
		}
		else if(!strncmp(cmd, "print", 5)){
			
			interest_print();
		}
		else if(!strncmp(cmd, "empty", 5)){
			
			interest_empty();
		}
	}
}
