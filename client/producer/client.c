#include "ndn.h"
#include "interest_queue.h"
#include "data_queue.h"
#include "key_queue.h"
#include <pthread.h>


#define DATA_RECV_TIME 60

static sockaddr_in router_addr;
static char router_ip[20];
static int router_port;


static sockaddr_in KGC_addr;
static char KGC_ip[20] = "127.0.0.1";
static char KGC_port[10] = "12000";


unsigned char user_identity[16];
RSA user_keys;
static int key_sock = -1;

int state = 0;
char client_cmd[7][16] = {"connect", "disconnect", "test", "request", "stop", "upload", "print"};
static int sock_fd = -1;
pthread_t RECV, SEND, TIME;
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;


void* PACKET_RECV(void* arg);

void* DATA_SEND(void* arg);

void* TIME_CHECK(void* arg);

void command();

int ndn_send(ndn_packet*, int);

void ndn_connect(char*, char*);

void ndn_disconnect();

void ndn_test();

void ndn_request(const char*, const char*);

void ndn_stop(char*, int);

void ndn_upload(const char*);

void ndn_print(char*);

void KGC_communicate();

void KEY_get(char*);

ndn_packet* SIGN(ndn_packet*);

int VERIFY(char*, ndn_packet);




int main(int argc, char** argv)
{
	if(argc == 4){
		
		strcpy(user_identity, argv[1]);
		
		addr_set(&router_addr, argv[2], argv[3]);
	}
	else{
		
		printf("FAIL TO RUN THE PROGRAM: LACK OF ARGUMENTS\n");
		return 0;
	}
	
	addr_set(&KGC_addr, KGC_ip, KGC_port);
	KGC_communicate();
	
	
	srand(time(NULL));
	
	
	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
	
	tcp_connect(router_addr, sock_fd);
	
	
	pthread_create(&RECV, NULL, PACKET_RECV, NULL);
	
	
	ndn_packet* connect_request = pack("", 4, 0, NULL);
	ndn_send(connect_request, sizeof(ndn_header));
	
	
	free(connect_request);
	
	command();
	
}




void* PACKET_RECV(void* arg)
{
	/*
	========================================================================================================================== RECVEIVE PACKET *THREAD*
	*
	*
	*
	===============================================================
	*/
	
	
	ndn_packet* packet = NULL;
	int recv_size;
	timeval now;
	
	
	while(1){
		
		packet = packet_rst(packet);
		
		
		recv_size = recv(sock_fd, packet, sizeof(ndn_packet), 0);
		//printf("recv_size = %d\n", recv_size);
		
		
		if(recv_size == -1){
			
			printf("\n.\n.\nReceiving Packet Failed\n.\n.\n");
			
			sleep(3);
		}
		else if(recv_size == 0){
			;
		}
		else{
			
			msg_print(router_addr, *packet);
			
			
			if(!state && packet->header.type == 5){
				/* success */
				
				state = 1;
				pthread_create(&SEND, NULL, DATA_SEND, NULL);
				pthread_create(&TIME, NULL, TIME_CHECK, NULL);
				
				printf(".\n.\nConnection Succeed\n.\n.\n\n");
			}
			else if(!state && packet->header.type == -1){
				/* fail */
				
				close(sock_fd);
				sock_fd = -1;
				printf(".\n.\nConnection Failed\n.\n.\n\n");
				
				pthread_exit(NULL);
			}
			else if(state && packet->header.type == 1){
				/* interest */
				
				char name[32] = "";
				int seg = 0;
				
				sscanf(packet->header.name, "%[^/]/%d", name, &seg);
				//printf("%s %s\n", name, seg);
				
				
				if(access(name, F_OK) != -1){
					
					pthread_mutex_lock(&MUTEX);
					
						data_enqueue(packet->header.name, name, seg);
					
					pthread_mutex_unlock(&MUTEX);
				}
				
				
			}
			else if(state && packet->header.type == 2){
				/* data */
				
				char name[32] = "";
				int seg = 0;
				
				sscanf(packet->header.name, "%[^/]/%d", name, &seg);
				
				pthread_mutex_lock(&MUTEX);
				
					INTEREST* t = interest_find(name);
					
				pthread_mutex_unlock(&MUTEX);
				
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////adjust condition contrl: v & t
				
				if(t && VERIFY(t->identity, *packet)){
					
					if(seg == t->segment){
						
						int cont_size = fwrite(packet->content, 1, packet->header.len, t->file);
						
						if(cont_size == MSS){
							
							t->segment++;
						}
						else if(cont_size < MSS){
							
							INTEREST* p = t;
							t = t->next;
							interest_dequeue(p);
						}
					}
					
					
					if(interest_head){
						
						char e_name[32] = "";
					
						sprintf(e_name, "%s/%d", interest_head->name, interest_head->segment);
					
						ndn_packet* packet = pack(e_name, 1, 0, NULL);
					
						ndn_send(packet, sizeof(ndn_header));
					
						free(packet);
					}
					
				}
				
				
				/*if(t && v && t == interest_head){
					
					char e_name[32] = "";
					
					sprintf(e_name, "%s/%d", t->name, t->segment);
					
					ndn_packet* packet = pack(e_name, 1, 0, NULL);
					
					ndn_send(packet, sizeof(ndn_packet));
					
					free(packet);
				}*/
			}
			else if(state && packet->header.type == 3){
				/* test */	
				;
			}
			else{
				/*unknown*/
				
				printf(".\n.\nUnknown packet\n.\n.\n\n");
			}
			//sleep(2);
		}
	}
	
	
}



void* DATA_SEND(void* arg)
{
	/*
	========================================================================================================================== SEND DATA *THREAD*
	*
	*
	*
	===============================================================
	*/
	
	
	ndn_packet* packet = NULL;
	FILE* file;
	char transmit_name[32] = "";
	char content[MSS];
	int file_size;
	int cont_size;
	
	
	while(1){
		
		if(data_head){
			
			file = fopen(data_head->name, "r");
			
			strcpy(transmit_name, data_head->entire_name);
			
			fseek(file, 0, SEEK_END);
			file_size = ftell(file);
			fseek(file, 0, SEEK_SET);
			
			
			while(data_head && !strcmp(transmit_name, data_head->entire_name)){
				
				packet = packet_rst(packet);
				memset(content, 0, MSS);
				
				if(file_size == data_head->segment*MSS){
					
					packet = pack(data_head->entire_name, 2, 0, NULL);
					packet = SIGN(packet);
				}
				else if(file_size > data_head->segment*MSS){
					
					if(ftell(file) != data_head->segment*MSS){
					
						fseek(file, data_head->segment*MSS, SEEK_SET);
					}
				
				
					cont_size = fread(content, 1, MSS>file_size?file_size:MSS, file);
				
					packet = pack(data_head->entire_name, 2, cont_size, content);
					packet = SIGN(packet);
				}
				
				int send_size = ndn_send(packet, sizeof(ndn_header) + sizeof(LINT) + cont_size);
				//printf("send_size = %d\n", send_size);
				
				
				pthread_mutex_lock(&MUTEX);
				
					data_dequeue();
				
				pthread_mutex_unlock(&MUTEX);
			}
			
			
			fclose(file);
			strcpy(transmit_name, "");
		}
	}
}



void* TIME_CHECK(void* arg)
{
	/*
	========================================================================================================================== CHECK TIME *THREAD*
	*
	*
	*
	===============================================================
	*/
	
	
	while(1){
		
		pthread_mutex_lock(&MUTEX);
			
			interest_lifetime_check();
			
		pthread_mutex_unlock(&MUTEX);
		
		sleep(1);
	}
}



void command()
{
	/*
	========================================================================================================================== command
	*
	*
	*
	===============================================================
	*/
	
	
	char cmd[64] = "";
	char action[10] = "";
	
	
	while(1){
		
		strcpy(cmd, "");
		strcpy(action, "");
		
		fgets(cmd, sizeof(cmd), stdin);
		
		if(strlen(cmd) >= strlen(client_cmd[0]) + 1 && !strncmp(cmd, client_cmd[0], strlen(client_cmd[0]))){
			/* connect */
			
			char ip[20] = "";
			char port[20] = "";
			
			if(sscanf(cmd, "%s %s %s\n", action, ip, port) == 3){
				
				ndn_connect(ip, port);
			}
			else{
				
				printf("command error: arguments lost\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[1]) + 1 && !strncmp(cmd, client_cmd[1], strlen(client_cmd[1]))){
			/* disconnect */
			
			if(sscanf(cmd, "%s\n", action) == 1){
				
				ndn_disconnect();
			}
			else{
				
				printf("command error: one argument needed\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[2]) + 1 && !strncmp(cmd, client_cmd[2], strlen(client_cmd[2]))){
			/* test */
			
			if(sscanf(cmd, "%s\n", action) == 1){
				
				ndn_test();
			}
			else{
				
				printf("command error: one argument needed\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[3]) + 1 && !strncmp(cmd, client_cmd[3], strlen(client_cmd[3]))){
			/* request */
			
			char identity[16] = "";
			char name[32] = "";
			
			if(sscanf(cmd, "%s %s %s\n", action, identity, name) == 3){
				
				ndn_request(identity, name);
			}
			else{
				
				printf("command error: arguments lost\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[4]) + 1 && !strncmp(cmd, client_cmd[4], strlen(client_cmd[4]))){
			/* stop */
			
			char name[32] = "";
			int nonce = 0;
			
			if(sscanf(cmd, "%s %s %d\n", action, name, &nonce) == 3){
				
				ndn_stop(name, nonce);
			}
			else{
				
				printf("command error: arguments lost\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[5]) + 1 && !strncmp(cmd, client_cmd[5], strlen(client_cmd[5]))){
			/* upload */
			
			char name[32] = "";
			
			if(sscanf(cmd, "%s %s\n", action, name) == 2){
				
				ndn_upload(name);
			}
			else{
				
				printf("command error: argument lost\n");
			}
		}
		else if(strlen(cmd) >= strlen(client_cmd[6]) + 1 && !strncmp(cmd, client_cmd[6], strlen(client_cmd[6]))){
			/* print */
			
			char type[10] = "";
			
			if(sscanf(cmd, "%s %s\n", action, type) == 2){
				
				ndn_print(type);
			}
			else{
				
				printf("command error: argument lost\n");
			}
		}
		else{
			/* unknown */	
			
			printf("command error: unknown command\n");
		}
	}
}



int ndn_send(ndn_packet* packet, int size)
{
	/*
	========================================================================================================================== send
	*
	*
	*
	===============================================================
	*/
	
	
	int send_size = 0;
	
	while(1){
	
		send_size = send(sock_fd, packet, size, 0);
		
		if(send_size == -1){
			
			printf(".\n.\nSending Packet Failed\n.\n.\n\n");
			sleep(3);
		}
		else{
			
			msg_print(router_addr, *packet);
			
			return send_size;
		}
	}
}



void ndn_connect(char* ip, char* port)
{
	/*
	========================================================================================================================== connect
	*
	*
	*
	===============================================================
	*/
	
	
	if(!state){
		
		addr_set(&router_addr, ip, port);
		
		if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			
			perror("Socket Creation Failed");
			exit(EXIT_FAILURE);
		}
		
		tcp_connect(router_addr, sock_fd);
		
		pthread_create(&RECV, NULL, PACKET_RECV, NULL);
		
		ndn_packet* packet = pack("", 4, 0, NULL);
		ndn_send(packet, sizeof(ndn_header));
		
		free(packet);
	}
	else{
		
		printf(".\n.\nConnection Failed: Already Connect to %s %d\n.\n.\n\n", inet_ntoa(router_addr.sin_addr), ntohs(router_addr.sin_port));
		
		return;
	}
}



void ndn_disconnect()
{
	/*
	========================================================================================================================== disconnect
	*
	*
	*
	===============================================================
	*/
	
	
	if(state){
		
		pthread_mutex_lock(&MUTEX);
			
			interest_empty();
			data_empty();
			
			pthread_cancel(RECV);
			
		pthread_mutex_unlock(&MUTEX);
		
		
		ndn_packet* packet = pack("", 6, 0, NULL);
		ndn_send(packet, sizeof(ndn_header));
		
		pthread_cancel(SEND);
		pthread_cancel(TIME);
		
		free(packet);
		
		close(sock_fd);
		sock_fd = -1;
		state = 0;
		
		printf(".\n.\nDisconnection Finished\n.\n.\n\n");
	}
	else{
		
		printf(".\n.\nDisconnection Failed: Disconnection Already Finished\n.\n.\n\n");
		
		return;
	}
}



void ndn_test()
{
	/*
	========================================================================================================================== test
	*
	*
	*
	===============================================================
	*/
	
	
	if(state){
		
		ndn_packet* packet = pack("", 3, 0, NULL);
		ndn_send(packet, sizeof(ndn_header));
		
		free(packet);
	}
	else{
		
		printf(".\n.\nTest Failed: Connection Lost\n.\n.\n\n");
		
		return;
	}
}



void ndn_request(const char* identity, const char* name)
{
	/*
	========================================================================================================================== request
	*
	*
	*
	===============================================================
	*/
	
	
	if(state){
		
		KEY_get(identity);
		
		char e_name[32] = "";
	
		if(!interest_head){
		
			sprintf(e_name, "%s/0", name);
	
			ndn_packet* packet = pack(e_name, 1, 0, NULL);
	
			ndn_send(packet, sizeof(ndn_header));
	
			free(packet);
		}
		
		
		pthread_mutex_lock(&MUTEX);
		
			interest_enqueue(identity, name, DATA_RECV_TIME);
		
		pthread_mutex_unlock(&MUTEX);
	}
	else{
		
		printf(".\n.\nRequest Failed: Connection Lost\n.\n.\n\n");
		
		return;
	}
}



void ndn_stop(char* name, int nonce)
{
	/*
	========================================================================================================================== stop
	*
	*
	*
	===============================================================
	*/
	
	
	INTEREST* t = interest_find(name);
	
	if(t){
		
		printf("Stop Receiving file...\n\n");
		
		interest_dequeue(t);
	}
}



void ndn_upload(const char* name)
{
	/*
	========================================================================================================================== upload
	*
	*
	*
	===============================================================
	*/
	
	
	if(state){
		
		if(access(name, F_OK) != -1){////////////////////////////////////////////////
			
			FILE* file = fopen(name, "r");
			
			int file_size;
			char seg[10] = "";
			char e_name[32] = "";
			
			fseek(file, 0, SEEK_END);
			file_size = ftell(file);
			fseek(file, 0, SEEK_SET);
			
			fclose(file);
			
			
			for(int i = 0; i <= file_size/MSS; i++){
				
				sprintf(e_name, "%s/%d", name, i);
				
				pthread_mutex_lock(&MUTEX);
				
					data_enqueue(e_name, name, i);
				
				pthread_mutex_unlock(&MUTEX);
				
				strcpy(e_name, "");
			}
		}
		else{
			
			printf(".\n.\nUpload Failed: Unknown Data\n.\n.\n\n");
		}
	}
}



void ndn_print(char* type)
{
	/*
	========================================================================================================================== print
	*
	*
	*
	===============================================================
	*/
	
	
	if(!strcmp("interest", type)){
		
		interest_print();
	}
	else if(!strcmp("data", type)){
		
		data_print();
	}
	else{
		
		printf(".\n.\nPrint Failed: Unknown List\n.\n.\n\n");
	}
}



void KGC_communicate()
{
	/*
	========================================================================================================================== KGC COMMUNICATE
	*
	*
	*
	===============================================================
	*/
	
	
	if((key_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
	tcp_connect(KGC_addr, key_sock);
	
	
	key_packet* packet = (key_packet*)malloc(sizeof(key_packet));
	memset(packet, 0, sizeof(key_packet));
	
	strcpy(packet->identity, user_identity);
	packet->secure = 1;
	
	
	int send_size = send(key_sock, packet, sizeof(int) + (sizeof(char)*16), 0);
	
	if(send_size == -1){
		
		printf(".\n.\nSending Packet Failed\n.\n.\n\n");
	}
	else{
		
		//printf("send_size = %d\n", send_size);
		key_msg_print(KGC_addr, *packet);
	}
	
	
	
	memset(packet, 0, sizeof(key_packet));
	
	int recv_size = recv(key_sock, packet, sizeof(key_packet), 0);
	
	if(recv_size == -1){
		
		printf(".\n.\nReceiving Packet Failed\n.\n.\n\n");
	}
	else if(recv_size == 0){
		;
	}
	else{
		
		key_msg_print(KGC_addr, *packet);
		
		memcpy(user_keys.e.t, packet->keys.e.t, sizeof(LINT));
		memcpy(user_keys.d.t, packet->keys.d.t, sizeof(LINT));
		memcpy(user_keys.n.t, packet->keys.n.t, sizeof(LINT));
	}
	
	close(key_sock);
	key_sock == -1;
}


void KEY_get(char* identity)
{
	/*
	========================================================================================================================== KEY GET
	*
	*
	*
	===============================================================
	*/
	
	
	if((key_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
	tcp_connect(KGC_addr, key_sock);
	
	
	key_packet* packet = (key_packet*)malloc(sizeof(key_packet));
	memset(packet, 0, sizeof(key_packet));
	
	strcpy(packet->identity, identity);
	packet->secure = 0;
	
	
	int send_size = send(key_sock, packet, sizeof(int) + (sizeof(char)*16), 0);
	
	if(send_size == -1){
		
		printf(".\n.\nSending Packet Failed\n.\n.\n\n");
	}
	else{
		
		key_msg_print(KGC_addr, *packet);
	}
	
	
	
	memset(packet, 0, sizeof(key_packet));
	
	int recv_size = recv(key_sock, packet, sizeof(key_packet), 0);
	
	if(recv_size == -1){
		
		printf(".\n.\nReceiving Packet Failed\n.\n.\n\n");
	}
	else if(recv_size == 0){
		;
	}
	else{
		
		key_msg_print(KGC_addr, *packet);
		
		if(recv_size == (sizeof(int) + sizeof(char)*16)){
			
			printf("Requesting Key Failed: key information of specific identity doesn't exit\n");
		}
		else{
			
			key_enqueue(packet->identity, packet->keys.d.t, packet->keys.n.t);
		}
		
		//printf("recv_size = %d\n", recv_size);
	}
	
	close(key_sock);
	key_sock = -1;
}


ndn_packet* SIGN(ndn_packet* packet)
{
	/*
	========================================================================================================================== SIGN
	*
	*
	*
	===============================================================
	*/
	
	char buffer[(sizeof(char)*32) + MSS];
	
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, packet->header.name, sizeof(char)*32);
	strcat(buffer, packet->content);
	
	//printf("name + content: %s\n", buffer);
	
	unsigned char hash[16];
	
	MD5_CTX md5;
	MD5Init(&md5);

	//printf("buffer: %d\n", strlen(buffer));
	MD5Update(&md5, buffer, strlen((char *)buffer));
	MD5Final(hash, &md5);
	
	
	LINT S = RSA_Encrypt(hash, 16, user_keys.e, user_keys.n);
	
	memcpy(packet->signature.t, S.t, sizeof(LINT));
	
	printf("SIGNNED.............................\n");
	
	return packet;
}


int VERIFY(char* identity, const ndn_packet packet)
{
	/*
	========================================================================================================================== VERIFY
	*
	*
	*
	===============================================================
	*/
	
	char buffer[(sizeof(char)*32) + MSS];
	
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, packet.header.name, sizeof(char)*32);
	strcat(buffer, packet.content);
	
	//printf("name + content: %s\n", buffer);
	
	unsigned char hash1[16];
	unsigned char hash2[16];
	
	MD5_CTX md5;
	MD5Init(&md5);

	//printf("buffer: %d\n", strlen(buffer));
	MD5Update(&md5, buffer, strlen((char *)buffer));
	MD5Final(hash1, &md5);
	
	
	KEY* t = key_find(identity);
	LINT public_key, n_key;
	memcpy(public_key.t, t->public_key, sizeof(LINT));
	memcpy(n_key.t, t->n_key, sizeof(LINT));
	
	
	RSA_Decrypt(hash2, 16, packet.signature, public_key, n_key);
	
	
	if(memcmp(hash1, hash2, 16) == 0){
		
		printf("VERIFIED............................\n");
		
		return 1;
	}
	else{
		
		printf("UNKNOWN SOURCE: FAILED TO VERIFY!!!!\n");
		
		return 0;
	}
}






/*
========================================================================================================================== question
	* 10/21
	*		what happen if thread RECV receive packets when thread mutex is locked
	*
	===============================================================
*/


