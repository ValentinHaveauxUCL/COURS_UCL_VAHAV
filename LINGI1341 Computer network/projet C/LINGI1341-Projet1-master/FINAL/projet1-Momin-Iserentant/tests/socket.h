#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#define TRUE 1
#define FALSE 0
#define MAX_BYTES_DATA 1024

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval){
	//Création des addrinfo de source et destination
	struct addrinfo hints, *res;
	memset (&hints, 0, sizeof(hints));

	//attribution des différents champs
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags=AI_CANONNAME;
	
	//récupération des informations
	int erreur = getaddrinfo(address, NULL, &hints, &res);
	if(erreur!=0){	
		return "ERROR: getaddrinfo finished unsuccessfully\n";
	}

	//création de l'adresse de retour
	struct sockaddr_in6 *add = (struct sockaddr_in6 *) res->ai_addr;
	
	//copie dans rval	
	memcpy((void *)rval,(void *)add,sizeof(*add));
	
	return NULL;
	
}


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr,
                 int src_port,
                 struct sockaddr_in6 *dest_addr,
                 int dst_port){
	//création du socket
	int udp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(udp_socket==-1){
		perror("ERROR: unable to create a socket\n");
		return -1;
	}

	//Binding si nécessaire à la source local.
	if(source_addr!=NULL){
		if(src_port>0){
			source_addr->sin6_port =(in_port_t) htons(src_port);
		} else {
			perror("ERROR: incorect number of port\n");
			return -1;
		}
		int test_bind = bind(udp_socket,(struct sockaddr *) source_addr, sizeof(*source_addr));
		if(test_bind!=0){
			perror("ERROR: unable to do 'bind()'\n");
			return -1;
		}	
	}

	//connexion si nécessaire à la source distante.
	if(dest_addr!=NULL){
		if(dst_port>0){
			dest_addr->sin6_port = (in_port_t) htons(dst_port);
		} else {
			perror("ERROR: incorect number of port\n");
			return -1;
		}
		int test_connect = connect(udp_socket, (struct sockaddr *) dest_addr,sizeof(*dest_addr));
		if(test_connect!=0){
			perror("ERROR: unable to do connect()\n");
			return -1;
		}		
	}	

	return udp_socket;
}


/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){
	int len =MAX_BYTES_DATA;
	char *buf = malloc(len*sizeof(char));
	struct sockaddr_in6 *addr = malloc(sizeof(struct sockaddr_in6));
	int length = sizeof(*addr);
	ssize_t erreur = recvfrom(sfd, buf, len, MSG_PEEK,(struct sockaddr *) addr, (socklen_t *) &length);
	if(erreur==-1){
		return -1;
	}
	erreur =  connect(sfd, (struct sockaddr *) addr,sizeof(*addr));
	free(buf);
	free(addr);
	return erreur;
}


#endif
