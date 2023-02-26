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


//establish tcp connection
void tcp_connect(sockaddr_in addr, int sockfd)
{
	if(connect(sockfd, (sockaddr*)&addr, sizeof(sockaddr_in)) < 0){
		perror("Connection Failed");
		exit(EXIT_FAILURE);
	}
}


//reset and clear data of the packet
ndn_packet* packet_rst(ndn_packet* packet) //return or not
{
	if(packet == NULL){
		packet = (ndn_packet*)malloc(sizeof(ndn_packet));
	}
	memset(packet, 0, sizeof(ndn_packet));
	return packet;
}


//pack up the packet
ndn_packet* pack(char* name, int type, int len, char* content)
{
	ndn_packet* packet = (ndn_packet*)malloc(sizeof(ndn_packet));
	
	
	strcpy(packet->header.name, name);
	
	packet->header.type = type;
	
	packet->header.nonce = random_number();
	
	gettimeofday(&(packet->header.time), NULL);
	
	packet->header.len = len;
	
	memcpy(packet->content, content, len);
	
	
	return packet;
}


//print the imformation of the packet
void msg_print(const sockaddr_in addr, const ndn_packet packet)
{
	char str[16] = "";
	
	printf("====================================\n");
	
	printf("Connect Address: %s %d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	switch(packet.header.type){
		case 1:
		printf("\033[0;31m");	//RED
		strcpy(str, "Interest");
		break;
		
		case 2:
		printf("\033[0;34m");	//BLUE
		strcpy(str, "Data    ");
		break;
		
		case 3:
		printf("\033[0;37m");	//GRAY
		strcpy(str, "Test    ");
		break;
		
		case 4:
		printf("\033[0;32m");	//GREEN
		strcpy(str, "Connect ");
		break;
		
		case 5:
		printf("\033[0;33m");	//YELLOW
		strcpy(str, "Success ");
		break;
		
		case 6:
		printf("\033[1;35m");
		strcpy(str, "Disconnect ");
		break;
		
		default:
		printf("printing error: type = %d\n", packet.header.type);
		return;
	}
	printf("**********%s  Detail**********\n", str);
	printf("     name: %s\n", packet.header.name);
	printf("    nonce: %d\n", packet.header.nonce);
	printf("     time: %d sec %d usec\n", packet.header.time.tv_sec, packet.header.time.tv_usec);
	printf("pack size: %d bytes\n", packet.header.len);
	
	printf("\033[0m");	//DEFAULT COLOR
	printf("====================================\n\n");
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

/*
void sockopt_set(int option, int sockfd)
{
	if(setsockopt(sockfd, SOL_SOCKET, SOL_REUSEADDR | SOL_REUSEPORT, &option, sizeof(option))){
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

void connect_listen(int sockfd)
{
	if(listen(sockfd, 5) < 0){
		perror("Pending Queue is Full");
		exit(EXIT_FAILURE);
	}
}
*/





