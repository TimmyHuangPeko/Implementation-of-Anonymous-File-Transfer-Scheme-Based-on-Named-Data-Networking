#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include "MD5.h"
#include "RSA.h"

#define MaxSize 16384

/*=======================

	NDN PACKET

=======================*/

struct Head{
	char name[32];
	int type;		//1:InterestPacket	2:DataPacket	3:TestPacket	4:Connect	5:Succes	-1:Errors
	int nonce;
	struct timeval time;
	int len;
};

struct InterestPacket{
	struct Head head;
	LINT signature;
	unsigned char Content[MaxSize];
};


