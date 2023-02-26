#ifndef NDN_H
#define NDN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include "RSA.h"
#include "MD5.h"




typedef struct key_packet{
	int secure;
	char identity[16];
	RSA keys;
}key_packet;


typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct timeval timeval;

// produce random number
unsigned int random_number();

//set up address including ip and port
void addr_set(sockaddr_in* addr, char* ip, char* port);

//create a new socket
int socket_create();

//reset and clear data of the packet
key_packet* packet_rst(key_packet* packet);

//pack up the packet
key_packet* pack(int secure, char* identity, RSA key);

//print the imformation of the packet
void key_msg_print(const sockaddr_in addr, const key_packet packet);

//set socket option
void sockopt_set(int option, int sockfd);

//bind socket to address
void socket_bind(sockaddr_in addr, int sockfd);

//listen to connect request
void connect_listen(int sockfd, int queue_size);

/*
struct Head{
	char name[32];
	int type;		//1:InterestPacket	2:DataPacket	3:TestPacket	4:Connect	5:Succes	-1:Errors
	int nonce;
	struct timeval time;
	int len;
};

struct InterestPacket{
	struct Head head;
	unsigned char Content[MaxSize];
};
*/


#endif
