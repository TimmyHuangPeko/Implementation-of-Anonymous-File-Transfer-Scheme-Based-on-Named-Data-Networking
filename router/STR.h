#include<stdio.h>
#include<stdlib.h>
#include<string.h>



int spilt(char** p, char* str){
	
	char* h = str;
	char* l = str + strlen(str);
	char* c;
	int i = 0;
	
	c = strchr(str, '/');
	
	while(c != 0 && c <= l){
		*(c++) = '\0';
		p[i++] = h;
		h = c;
		c = strchr(h, '/');
		
		printf("%s ",p[i-1]);
		
	}
	
	p[i] = h;
	
	printf("%s ",p[i]);
	
	printf("\n");
	
	return i;
}
