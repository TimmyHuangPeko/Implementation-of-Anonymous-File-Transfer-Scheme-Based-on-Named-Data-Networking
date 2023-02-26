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


#define MSS 16384

typedef struct ndn_header{
	char name[32];		//packet's name
	int type;			//including (interest packet = 1, data packet = 2, test packet = 3, connect = 4, success = 5, disconnect = 6, error = -1)
	unsigned int nonce;			//to detect if packets came from same source
	struct timeval time;	//record time
	int len;			//packet's size, excluding header's size
}ndn_header;
/*
the structure of a ndn header includes following 5 parts:
	1. name: data's name.
	2. type: including 5 types, all of them represent different purposes.
		(1) - interest packet, sent from clients.
		(2) - data packet, sent from servers.
		(3) - test packet, testing if the packet is delivered correctly.
		(4) - connect pakcet, clients and servers would send this type of packet to routers, to require connection between them
		(5) - success packet, if connection or disconnection between users and routers are succeed and finished, routers would send this type of packet to users
		(6) - disconnect packet, users send this type of packet to routers, to warn of disconnection
	3. nonce: a random number, to identify different user and different request
	4. time: record the time of sending packet
	5. len: the size of ndn packet's content
*/


typedef struct ndn_packet{
	ndn_header header;
	LINT signature;
	unsigned char content[MSS];
}ndn_packet;
/*
the structure of ndn packet includes ndn header and content of packet
*/

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

//establish tcp connection
void tcp_connect(sockaddr_in addr, int sockfd);

//reset and clear data of the packet
ndn_packet* packet_rst(ndn_packet* packet);

//pack up the packet
ndn_packet* pack(char* name, int type, int len, char* content);

//print the imformation of the packet
void msg_print(const sockaddr_in addr, const ndn_packet packet);

void key_msg_print(const sockaddr_in addr, const key_packet packet);

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
