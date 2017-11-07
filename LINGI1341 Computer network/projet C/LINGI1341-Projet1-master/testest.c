#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>

int main(int argc, const char *argv[]){
	int i=0;
	char bruffer[1024];
	char lecture[1024];
	int bo = 1;
	while(bo){
	read(STDIN_FILENO, (void *) lecture, 1024);
	printf("LECTURE: %s",lecture);
	printf("size= %d\n", (int) strlen(lecture));
	if(strcmp(lecture, "/stop")!=0){
	memcpy((void *) &bruffer[i], (void *) lecture, (int) strlen(lecture));
	i=i+(int) strlen(lecture);
	printf("I=%d\n", i);
	
	printf("BUFFER: %s",bruffer);
	} else {
		bo=0;
	}
	}
}
