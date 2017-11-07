#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <fcntl.h>


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(const struct sockaddr_in6 *source_addr,
                 int src_port,
                 const struct sockaddr_in6 *dest_addr,
                 int dst_port){

	int udp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(udp_socket==-1){
		perror("ERROR: unable to create a socket\n");
		return -1;
	}
	if(source_addr!=NULL){
		struct sockaddr_in6 *source = malloc(sizeof(*source_addr));
		memcpy((void *)source_addr, (void *) source, sizeof(*source_addr));
		if(src_port>0){
			source->sin6_port =htons(src_port);
			source->sin6_family = AF_INET6;
		} else {
			perror("ERROR: incorect number of port\n");
			return -1;
		}
		int test_bind = bind(udp_socket,(struct sockaddr *) source, sizeof(*source));
		if(test_bind==-1){
			perror("ERROR: unable to do 'bind()'\n");
			return -1;
		}	
	}
	if(dest_addr!=NULL){
		struct sockaddr_in6 *dest=malloc(sizeof(*dest_addr));
		memcpy((void *) dest_addr, (void *) dest, sizeof(*dest_addr));
		if(dst_port>0){
			dest->sin6_port = htons(dst_port);
			dest->sin6_family = AF_INET6;
		} else {
			perror("ERROR: incorect number of port\n");
			return -1;
		}
		int test_connect = connect(udp_socket, (struct sockaddr *) dest,sizeof(*dest));
		if(test_connect==-1){
			perror("ERROR: unable to do 'connect()'");
			return -1;
		}		
	}	
	return udp_socket;
}

int main(int argc, const char *argv[]){
	return 0;
}
