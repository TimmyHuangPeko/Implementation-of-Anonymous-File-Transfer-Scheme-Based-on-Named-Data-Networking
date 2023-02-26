#include <stdio.h>

int main(void)
{
	/*
	char name[32] = "";
	scanf("%s", name);
	printf("%s\n", name);
	
	char n[32] = "";
	char s[10] = "";
	sscanf(name, "%[^/]/%s", n , s);
	printf("%s\n%s\n", n, s);
	
	int seg = 0;
	
	seg = (int)strtol(s, NULL, 10);
	printf("%d\n", seg);
	*/
	
	int i;
	char number[10] = "";
	char result[32] = "";
	
	while(1){
		
		scanf("%s %d", number, &i);
		sprintf(result, "%s/%d", number, i);
		printf("%s\n", result);
	}
	
	return 0;
}
