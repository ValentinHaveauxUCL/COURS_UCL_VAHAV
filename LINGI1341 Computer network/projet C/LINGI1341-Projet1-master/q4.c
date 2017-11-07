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
/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){
	int len =512;
	char *buf = malloc(len*sizeof(char));
	struct sockaddr_in6 *addr = malloc(sizeof(struct sockaddr_in6));
	int length = sizeof(*addr);
	ssize_t erreur = recvfrom(sfd, buf, len, 0,(struct sockaddr *) addr, (socklen_t *) &length);
	if(erreur==-1){
		return -1;
	}
	erreur =  connect(sfd, (struct sockaddr *) addr,sizeof(*addr));
	return erreur;
}

int main(int argc, const char *argv[]){
 return 0;
}
