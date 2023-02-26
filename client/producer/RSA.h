#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>

#define SIZE 512
#define P_N 3
#define MAX_P 1000000
#define MAX_K 1000000
#define MIN_K 12
#define MAX_N 9
#define MAX_T 100000



typedef struct{
	unsigned char t[SIZE];
	//int size;
}LINT;

typedef struct{
	LINT e;
	LINT d;
	LINT n;
}RSA;


void print(LINT x);
RSA Creat_Key();
LINT RSA_Encrypt(unsigned char* mes, int size, LINT e, LINT n);
void RSA_Decrypt(unsigned char* mes, int size, LINT e, LINT d, LINT n);
