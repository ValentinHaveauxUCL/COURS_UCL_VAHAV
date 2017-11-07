#include "read_write_loop.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_SEGMENT_SIZE 1024

void read_write_loop(int sfd) {

	fd_set sfds;
    char buf[MAX_SEGMENT_SIZE];
	FD_ZERO(&sfds);

	while(true)
	{
        FD_SET(STDIN_FILENO,&sfds);
        FD_SET(sfd,&sfds);

    	select(sfd+1,&sfds,NULL,NULL,NULL);

    	if(FD_ISSET(STDIN_FILENO,&sfds))
    	{
            ssize_t sizeR=read(STDIN_FILENO,buf,MAX_SEGMENT_SIZE);
        	if(sizeR==EOF) break;
            write(sfd,buf,sizeR);
    	}
        else if(FD_ISSET(sfd,&sfds))
    	{
        	ssize_t sizeR=read(sfd,buf,MAX_SEGMENT_SIZE);
        	if(sizeR==EOF) break;
            write(STDOUT_FILENO,buf,sizeR);
    	}
	}
}
