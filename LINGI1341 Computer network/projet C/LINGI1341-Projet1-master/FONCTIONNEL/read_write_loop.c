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
	memset((void *) read_buffer, '\0', 1024);
	memset((void *) write_buffer, '\0', 1024);
	int datain_w = FALSE;
	int nbr_elem=0;

	struct timeval *timer = malloc(sizeof(struct timeval));
	timer->tv_sec=5;
	timer->tv_usec=0;

	fd_set *ecriture = malloc(sizeof(fd_set));
	FD_ZERO(ecriture);
	FD_SET(sfd, ecriture);

	fd_set *lecture = malloc(sizeof(fd_set));
	FD_ZERO(lecture);
	FD_SET(sfd, lecture);
	FD_SET(STDIN_FILENO, lecture);

	int envoi;	
	int reception;

	int boolean = TRUE;
	while(boolean){
		FD_ZERO(ecriture);
		FD_SET(sfd, ecriture);
		FD_ZERO(lecture);
		FD_SET(sfd, lecture);
		FD_SET(STDIN_FILENO, lecture);
		int try = select(sfd+1,lecture,ecriture,NULL,timer);
		if(try<0){
			//
		} else {
			if(FD_ISSET(sfd, lecture)){
				read(sfd, (void *) read_buffer,1024);
				printf("[Reçu] = %s", read_buffer);
				//write(STDOUT_FILENO, (void *) read_buffer, 1024);
			        memset((void *) read_buffer, '\0', 1024);
				try=-1;
			}
			if(FD_ISSET(sfd, ecriture) && datain_w){
				int length = strlen((const char *) write_buffer);
				if(write_buffer[length-1]=='\0'){
					boolean = FALSE;
				}
				write(sfd, (void *) write_buffer, 1024);
				memset((void *) write_buffer, '\0', 1024);
				datain_w=FALSE;
				try=-1;
			}
			if(FD_ISSET(STDIN_FILENO, lecture)){
				read(STDIN_FILENO, (void *) write_buffer, 1024);
				//printf("[Envoyé] = %s", write_buffer);
				datain_w=TRUE;
				try=-1;
			}
		}
	}
	
}
