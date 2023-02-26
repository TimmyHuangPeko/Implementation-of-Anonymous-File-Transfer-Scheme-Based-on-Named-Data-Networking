#include "data_queue.h"

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
			
			t = sscanf(cmd, "%s %s %d\n", action, name, &nonce);
			printf("%s %d\n",name, nonce);
			
			data_enqueue(name, nonce);
		}
		else if(!strncmp(cmd, "dequeue", 7)){
			
			char name[32] = "";
			int nonce;
			
			data_dequeue();
		}
		else if(!strncmp(cmd, "print", 5)){
			
			data_print();
		}
		else if(!strncmp(cmd, "empty", 5)){
			
			data_empty();
		}
	}
}
