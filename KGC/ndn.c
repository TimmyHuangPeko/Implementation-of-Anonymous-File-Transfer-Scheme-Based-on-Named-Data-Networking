#include "ndn.h"


// produce random number
unsigned int random_number()
{
	//srand(time(NULL));
	return rand() + 1;
}


//set up address including ip and port
void addr_set(sockaddr_in* addr, char* ip, char* port)
{
	memset((void*)addr, 0, sizeof(sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons((int)strtol(port, NULL, 10));
	addr->sin_addr.s_addr = inet_addr(ip);
}


//create a new socket
int socket_create()
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}


//reset and clear data of the packet
key_packet* packet_rst(key_packet* packet) //return or not
{
	if(packet == NULL){
		packet = (key_packet*)malloc(sizeof(key_packet));
	}
	memset(packet, 0, sizeof(key_packet));
	return packet;
}


//pack up the packet
key_packet* pack(int secure, char* identity, RSA key)
{
	key_packet* packet = (key_packet*)malloc(sizeof(key_packet));
	
	packet->secure = secure;
	
	strcpy(packet->identity, identity);
	
	memcpy(packet->keys.e.t, key.e.t, SIZE);
	
	memcpy(packet->keys.d.t, key.d.t, SIZE);
	
	memcpy(packet->keys.n.t, key.n.t, SIZE);
	
	return packet;
}



void key_msg_print(const sockaddr_in addr, const key_packet packet)
{
	
	printf("====================================\n");
	
	printf("Connect Address: %s %d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	
	if(packet.secure == 0){
		
		printf("**********key     Detail**********\n");
	}
	else{
		
		printf("**********Secure Channel**********\n");
	}
	
	printf(" identity: %s\n", packet.identity);
	
	printf("  private: ");
	print(packet.keys.e);
	printf("   public: ");
	print(packet.keys.d);
	printf("    n key: ");
	print(packet.keys.n);
	
	
	printf("====================================\n\n");
}


void sockopt_set(int option, int sockfd)
{
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))){
		perror("Socket Option Setting Failed");
		exit(EXIT_FAILURE);
	}
}


void socket_bind(sockaddr_in addr, int sockfd)
{
	if(bind(sockfd, (sockaddr*)&addr, sizeof(sockaddr_in)) < 0){
		perror("Socket Bind Failed");
		exit(EXIT_FAILURE);
	}
}


void connect_listen(int sockfd, int queue_size)
{
	if(listen(sockfd, queue_size) < 0){
		perror("Pending Queue is Full");
		exit(EXIT_FAILURE);
	}
}

















