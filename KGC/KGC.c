#include "ndn.h"
#include "key_queue.h"
#include "MD5.h"
#include "RSA.h"


sockaddr_in KGC_addr;
char KGC_ip[20] = "127.0.0.1";
char KGC_port[10] = "12000";
int recep_sock = -1;
int connect_sock = 0;

sockaddr_in client_addr;

int main(void)
{
	int addrlen = sizeof(sockaddr_in);
	
	
	addr_set(&KGC_addr, KGC_ip, KGC_port);
	
	if((recep_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
	int opt = 1;
	
	if(setsockopt(recep_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		
		perror("Socket Option Setting Failed");
		exit(EXIT_FAILURE);
	}
	
	if(bind(recep_sock, (sockaddr*)&KGC_addr, sizeof(sockaddr_in)) < 0){
		
		perror("Socket Bind Failed");
		exit(EXIT_FAILURE);
	}
	
	if(listen(recep_sock, 5) < 0){
	
		perror("Pending Queue is Full");
		exit(EXIT_FAILURE);
	}
	
	key_packet* recv_packet = NULL;
	key_packet* send_packet = NULL;
	int recv_size = 1;
	
	
	while(1){
		
		if((connect_sock = accept(recep_sock, (sockaddr*)&client_addr, (socklen_t*)&addrlen)) < 0){
		
			perror("Acception Failed\n");
			exit(EXIT_FAILURE);
		}
		
		while(recv_size > 0){
			
			recv_packet = packet_rst(recv_packet);
			send_packet = packet_rst(send_packet);
			
			
			recv_size = recv(connect_sock, recv_packet, sizeof(key_packet), 0);
			//printf("recv_size = %d\n", recv_size);
			if(recv_size <= 0){
				
				break;
			}
			else{
				
				KEY* t = NULL;
				
				
				key_msg_print(client_addr, *recv_packet);
				
				
				if(recv_packet->secure && !key_find(recv_packet->identity)){
					
					RSA new_key;
					new_key = Creat_Key();
					
					key_enqueue(recv_packet->identity, new_key);
					
					//key_print();///////////////////////////////////////////////////////////
					
					send_packet = pack(recv_packet->secure, recv_packet->identity, new_key);
				}
				else if(!recv_packet->secure && (t = key_find(recv_packet->identity))){
					
					RSA user_key;
					
					memset(user_key.e.t, 0, sizeof(LINT));
					memcpy(user_key.d.t, t->keys.d.t, sizeof(LINT));
					memcpy(user_key.n.t, t->keys.n.t, sizeof(LINT));
					
					send_packet = pack(recv_packet->secure, t->identity, user_key);
				}
				
				
				int send_size = send(connect_sock, send_packet, send_packet, sizeof(key_packet));
				if(send_size == -1){
					
					printf(".\n.\nSending Packet Failed\n.\n.\n\n");
				}
				else{
					
					key_msg_print(client_addr, *send_packet);
				}
			}
		}
		
		printf("========================Client Disconnect========================\n\n");
		connect_sock = 0;
		recv_size = 1;
	}
}



















