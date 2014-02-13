#include <stdio.h>
#include "EasyTHPS.h"

char str[1000];
char *ptr = "THREAD";
int get_thread_num(){
	FILE *fp = fopen("Config.mk","r");
    while(1)
	{
		fscanf(fp,"%s",str);
		if(strcmp(str,ptr) == 0)
			break;
	}	
	fscanf(fp,"%s",str);
	int res;
	fscanf(fp,"%d",&res);
	fclose(fp);
	return res;
}
