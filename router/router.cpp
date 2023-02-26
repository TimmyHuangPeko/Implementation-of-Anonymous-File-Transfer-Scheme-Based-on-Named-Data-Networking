#include "Stack.h"
#include "STR.h"

#define NUM 8
#define LEN 100
#define InterestTime 10
#define DataTime 1000


int ListenFd, newfd;
struct sockaddr_in srv;
char message[LEN];
int srv_len = sizeof(srv);
int nbyte;
bool TimeOff = 0;

int FD[NUM];
int FdMode[NUM];	// 1:Connect -1:No

Node stack;
Node CS;

pthread_t CMD, Time, FD_t[NUM];
pthread_mutex_t StackMutex = PTHREAD_MUTEX_INITIALIZER;


void build(char** argv);
void Listen();
void Implement(char* cmd);
void Test(int PORT);
void Connect(char* host, int port);
void Search(int fd, char* name);
void Give(int fd, char* name , char* data);
void* Command(void* num);
void* CONNECT(void* num);
void* CheckTime(void* num);

int main(int argc, char**argv){
	
	timeval tv;
	gettimeofday(&tv,NULL);
	//srand(tv.tv_sec + tv.tv_usec);
	srand(time(0));
	
	stack.next = 0;
	CS.next = 0;
	
	for(int a=0 ; a<NUM ; a++){
		FdMode[a] = -1;
	}
	
	pthread_create(&Time, NULL, CheckTime, NULL);
	pthread_create(&CMD, NULL, Command, NULL);
	
	build(argv);
	Listen();
}

void build(char** argv){

	/*==========================
	
		Build Server
		
	==========================*/
	
	ListenFd = socket(AF_INET, SOCK_STREAM, 0);
	
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( bind(ListenFd, (struct sockaddr*) &srv, sizeof(srv)) < 0 ){
		printf("BIND Error!!!\n");
		exit(1);
	}
	
	if(listen(ListenFd, 10) < 0){
		printf("LISTEN Error!!!\n");
		exit(1);
	}
	
}

void Listen(){

	/*==========================
	
		Router listen
		
	==========================*/
	
	struct InterestPacket GetPacket, SendPacket;
	
	while(1){
	
		if( (newfd = accept(ListenFd, (struct sockaddr*) &srv, (socklen_t *) &srv_len)) < 0){
			printf("accept ERROR!!!\n");
			continue;
		}
		
		nbyte = read(newfd, &GetPacket, sizeof(GetPacket));
		if(nbyte <= 0){
			printf("Disconnect\n");
			continue;
		}
		
		printf("Connect: %s\n", GetPacket.Content);
		
		if(GetPacket.head.type == 4){
			int a;
			for(a = 0; a < NUM; a++){
				if(FdMode[a] == -1){
					FD[a] = newfd;
					FdMode[a] = 1;
					int A = a;
					SendPacket.head.type = 5;
					write(FD[a], &SendPacket, sizeof(Head));
					pthread_create(&FD_t[a], NULL, CONNECT, &A);
					break;
				}
			}
			if(a == NUM){
				struct InterestPacket SendPacket;
				SendPacket.head.type = -1;
				write(newfd, &SendPacket, sizeof(Head));
			}
		}
		
	}
}

void Implement(char* cmd){

	/*==========================
	
		handle command
		
	==========================*/
	
	
	char instruct[LEN] = "";
	sscanf(cmd, "%s", instruct);
	
	
	if(!strcmp(instruct, "test")){
	
		int port = -1;
		sscanf(cmd, "%s %d", instruct, &port);
		if(port != -1){
			Test(port);
		}
		else{
			printf("Command Error!!!\n");
		}
		
	}
	else if(!strcmp(instruct, "connect")){
	
		char host[LEN] = "";
		int port = -1;
		sscanf(cmd, "%s %s %d", instruct, host, &port);
		if(strcmp(host, ""), port != -1){
			Connect(host, port);
		}
		else{
			printf("Command Error!!!\n");
		}
		
	}
	else if(!strcmp(instruct, "port")){
	
		printf("PORT : ");
		for(int a=0 ; a<NUM ; a++){
			if(FdMode[a] == 1)
				printf("%d ", a);
		}
		printf("\n");
		
	}
	else if(!strcmp(instruct, "search")){
	
		int fd = -1;
		char name[LEN] = "";
		sscanf(cmd, "%s %d %s", instruct, &fd, name);
		Search(fd, name);
		
	}
	else if(!strcmp(instruct, "print")){
	
		Print(&stack, &CS);
		
	}
	else if(!strcmp(instruct, "give")){
		
		int fd = -1;
		char name[LEN] = "";
		char data[LEN] = "";
		
		sscanf(cmd, "%s %d %s", instruct, &fd, name);
		
		
		fgets(data, sizeof(data), stdin);
		
		Give(fd, name, data);
		
		
	}
	else if(!strcmp(instruct, "time")){
		TimeOff = !TimeOff;
		
		if(TimeOff)
			printf("Time Off\n");
		else
			printf("Time On\n");
		
	}
	else{
		printf("Error Command!!!\n");
	}
	
}

void Test(int PORT){

	/*==========================
	
		Send Packet
		
	==========================*/
	
	
	if(PORT >= NUM && PORT < 0 && FdMode[PORT] == -1){
		printf("No Connect!!!\n");
		return;
	}
	
	struct InterestPacket SendPacket;
	strcpy((char*)SendPacket.Content, "Test");
	SendPacket.head.len = 5;
	SendPacket.head.type = 3;
	
	write(FD[PORT], &SendPacket, sizeof(Head) + SendPacket.head.len);
	
}

void Connect(char* host, int port){

	/*===================================
	
		Connect to other router
		
	===================================*/
	
	int a;
	for(a = 0 ; a < NUM ; a++){
		if( FdMode[a] == -1 ){
			break;
		}
	}
	if(a == NUM){
		printf("No PORT!!!\n");
		return;
	}
	
	
	int SendFd;
	int n;
	struct sockaddr_in cli;
	struct InterestPacket TestPacket, GetPacket;
	
	SendFd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(SendFd <= 0){
		printf("SOCKET ERROR!!!\n");
		exit(1);
	}
	
	cli.sin_family = AF_INET;
	cli.sin_port = htons(port);
	cli.sin_addr.s_addr = inet_addr(host);
	
	if( connect(SendFd, (struct sockaddr*) &cli, sizeof(cli)) < 0 ){
		printf("Connect Error!!!\n");
		exit(1);
	}
	
	strcpy((char*)TestPacket.Content, "Connect");
	TestPacket.head.len = strlen((char*)TestPacket.Content) + 1;
	TestPacket.head.type = 4;
	
	write(SendFd, &TestPacket, sizeof(struct Head) + TestPacket.head.len);
	read(SendFd, &GetPacket, sizeof(GetPacket));
	
	if(GetPacket.head.type == -1){
		printf("No Connect!!!\n");
	}
	else if(GetPacket.head.type == 5){
		printf("Connect Succes -> PORT[%d]\n", a);
		FD[a] = SendFd;
		FdMode[a] = 1;
		pthread_create(&FD_t[a], NULL, CONNECT, &a);
	}
	
}

void Search(int fd, char* name){

	/*===================================
	
		Throw InterestPacket from Stack
	
	===================================*/

	
	if( fd == -1 || fd < 0 || fd >= NUM || FdMode[fd] != 1 || !strcmp(name, "") ){
		printf("Port no open!!!\n");
		return;
	}
	
	struct InterestPacket SendPacket;
	strcpy(SendPacket.head.name, name);
	SendPacket.head.nonce = rand();
	SendPacket.head.type = 1;
	SendPacket.head.len = 0;
	gettimeofday(&SendPacket.head.time,NULL);
	write(FD[fd], &SendPacket, sizeof(Head));
	
	printf("SEND Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", fd, SendPacket.head.name, SendPacket.head.type, SendPacket.head.nonce, SendPacket.head.len);	
	
	pthread_mutex_lock(&StackMutex);
	insert(&stack, SendPacket, -1);
	pthread_mutex_unlock(&StackMutex);
}

void Give(int fd, char* name , char* data){

	/*===========================
	
		Give DataPacket
	
	===========================*/

	
	if(fd == -1 || fd < 0 || fd >= NUM || FdMode[fd] != 1 || !strcmp(name, "")){
		printf("Command Error!!!\n");
		return;
	}
	
	struct InterestPacket SendPacket;
	
	strcpy(SendPacket.head.name, name);
	strcpy((char*)SendPacket.Content, data);
	SendPacket.head.type = 2;
	SendPacket.head.nonce = rand();
	SendPacket.head.len = strlen(data) + 1;
	gettimeofday(&SendPacket.head.time,NULL);
	
	write(FD[fd], &SendPacket, sizeof(Head) + sizeof(LINT) + SendPacket.head.len);
	
	printf("SEND Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", fd, SendPacket.head.name, SendPacket.head.type, SendPacket.head.nonce, SendPacket.head.len);
	
	pthread_mutex_lock(&StackMutex);
	insert(&CS, SendPacket, -1);
	pthread_mutex_unlock(&StackMutex);
	
}



void* Command(void* num){

	/*==========================
	
		Insert Command
		
	==========================*/
	
	char cmd[LEN];
	while(1){
		fgets(cmd, sizeof(cmd), stdin);
		printf("Command : %s\n", cmd);
		
		Implement(cmd);
		
	}
	
	return 0;
}

void* CONNECT(void* num){

	/*======================================
	
		Connect with other server
	
	======================================*/
	
	int i = *(int*) num;
	int n;
	FdMode[i] = 1;
	
	printf("Connect Port[%d]\n", i);
	
	struct InterestPacket SendPacket, GetPacket;
	
	
	while(1){
		n = read(FD[i], &GetPacket, sizeof(GetPacket));
		//printf("N = %d\n", n);
		if(n <= 0){
			printf("PORT[%d] disconnect!!!\n", i);
			FdMode[i] = -1;
			break;
		}
		
		printf("GET Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", i, GetPacket.head.name, GetPacket.head.type, GetPacket.head.nonce, GetPacket.head.len);	
		
		if(GetPacket.head.type == 1){
			
			//InterestPacket
			
			pthread_mutex_lock(&StackMutex);
			
			
			if(Nonce(&stack, GetPacket)){
				printf("Drop Packet!!!\n");
				pthread_mutex_unlock(&StackMutex);
				continue;
			}
			
			
			
			if(DataGive(&CS, FD[i], GetPacket.head.name, i)){
				pthread_mutex_unlock(&StackMutex);
				continue;
			}
			
			insert(&stack, GetPacket, i);
			pthread_mutex_unlock(&StackMutex);
			
			
			for(int a = 0 ; a < NUM ; a++){
				if(a != i && FdMode[a] == 1){
					write(FD[a], &GetPacket, sizeof(Head));
					printf("SEND Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", a, GetPacket.head.name, GetPacket.head.type, GetPacket.head.nonce, GetPacket.head.len);
				}
			}
			
		}
		else if(GetPacket.head.type == 2){
			
			//DataPacket
			
			Node* data = 0;
			
			pthread_mutex_lock(&StackMutex);
			
			if(Nonce(&CS, GetPacket)){
				printf("Drop Packet!!!\n");
				pthread_mutex_unlock(&StackMutex);
				continue;
			}
					
			insert(&CS, GetPacket, i);
			
			Node* c = stack.next;
			
			while(c != 0){
				if( (c->num != -1 && c->Packet.head.type == 1 && !strcmp(c->Packet.head.name, GetPacket.head.name)) && c->num != -1 ){
					write(FD[c->num], &GetPacket, sizeof(Head) + sizeof(LINT) + GetPacket.head.len);
					printf("SEND Packet : FD:%d Name:%s Type:%d Nonce:%d Len:%d\n", c->num, GetPacket.head.name, GetPacket.head.type, GetPacket.head.nonce, GetPacket.head.len);
				}
				c = c->next;
			}
			
			pthread_mutex_unlock(&StackMutex);
			
		}
		else if(GetPacket.head.type == 3){
			
			//TestPacket
			
			printf("Test Packet from PORT[%d]\n", i);
			
			/*for(int a = 0 ; a < NUM ; a++){
				if(a != i && FdMode[a] != -1){
					write(FD[a], &GetPacket, sizeof(Head) + GetPacket.head.len);
				}
			}*/
		}
		else if(GetPacket.head.type == 6){
			
			//Disconnect Packet
			
			printf("PORT[%d] Disconnect!\n", i);
			
			close(FD[i]);
			FdMode[i] = -1;
			break;
			
		}
		
		
	}
	
	
	return 0;
}

void* CheckTime(void* num){

	/*===================================
	
		Check All Overtime Packet
	
	===================================*/

	TimeOff = 1;
	
	while(1){
		
		if(TimeOff){
			continue;
		}
		
		struct timeval time;
		gettimeofday(&time, NULL);
		
		pthread_mutex_lock(&StackMutex);
		TimePop(&stack, time);
		pthread_mutex_unlock(&StackMutex);
		
		sleep(1);
	}
	return 0;
}
