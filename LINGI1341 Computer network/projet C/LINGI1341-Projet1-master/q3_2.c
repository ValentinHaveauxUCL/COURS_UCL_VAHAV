#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
#define MAX_BYTES_DATA 1024

/* Loop reading a socket and printing to stdout,
 * while reading stdin and writing to the socket
 * @sfd: The socket file descriptor. It is both bound and connected.
 * @return: as soon as stdin signals EOF
 */
void read_write_loop(int sfd){
	char read_buffer[1024];
	char write_buffer[1024];
	int datain_w = FALSE;


	struct timeval *timer_envoi = malloc(sizeof(struct timeval));
	timer_envoi->tv_sec=5;
	timer_envoi->tv_usec=0;

	struct timeval *timer_reception = malloc(sizeof(struct timeval));
	timer_reception->tv_sec=5;
	timer_reception->tv_usec=0;

	fd_set *stdin_fd = malloc(sizeof(fd_set));
	FD_ZERO(stdin_fd);	
	FD_SET(STDIN_FILENO, stdin_fd);

	fd_set *socket_fd = malloc(sizeof(fd_set));
	FD_ZERO(socket_fd);
	FD_SET(sfd, socket_fd);

	int envoi;	
	int reception;

	int boolean = TRUE;
	while(boolean){
		printf("[STATUS]readSTDIN\n");
		read(STDIN_FILENO, (void *) write_buffer, 1024);
		datain_w=TRUE;
		envoi = select(sfd+1,NULL,socket_fd,NULL,timer_envoi);
		if(envoi<0){
			reception = select(sfd+1, socket_fd, NULL, NULL, timer_reception);
			if(reception<0){
				printf("BUG....\n");
			} else {
				if(FD_ISSET(sfd, socket_fd)){ 
					printf("[STATUS]REC socket\n");
					read(sfd, read_buffer,1024);
					printf("[Recu]: %s",read_buffer);
				}
			}
		} else {
			if(FD_ISSET(sfd, socket_fd) && datain_w){
				printf("[STATUS]readsocket\n");
				int length = strlen((const char *) write_buffer);
				if(write_buffer[length]=='\0'){
					boolean = FALSE;
				}
				write(sfd, (void *) write_buffer, 1024);
				datain_w=FALSE;
			}
		}		
	}
	
}

int main(int argc, const char *argv[]){
	
	
}



