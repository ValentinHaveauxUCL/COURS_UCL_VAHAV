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
//
#define TRUE 1
#define FALSE 0
#define MAX_BYTES_DATA 1024
#define BUF_SIZE 33

	char *int2bin(int a, char *buffer, int buf_size) {
    buffer += (buf_size - 1);
	int i;
    for (i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}

	char * getHeaderToBits(int a, int b, int c, int d){
	char buffer[BUF_SIZE];
    	buffer[BUF_SIZE - 1] = '\0';

	//type
	int value = a;
	int Type = value << 29;
	int2bin(Type, buffer, BUF_SIZE - 1);
        printf("Type = %s \n", buffer);

	//Window
	int value_window = b;
	int Window = value_window << 24;
	int2bin(Window, buffer, BUF_SIZE - 1);
        printf("Wind = %s \n", buffer);

	//Seqnum
	int value_seqnum = c;
	int Seqnum = value_seqnum << 16;
	int2bin(Seqnum, buffer, BUF_SIZE - 1);
        printf("seqnum = %s \n", buffer);

	//Length
	int value_length = d;
	int Length = value_length;
	int2bin(Length, buffer, BUF_SIZE - 1);
        printf("length = %s \n", buffer);

	int alt1 = Type ^ Window;
	int2bin(alt1, buffer, BUF_SIZE - 1);
        printf("alt1 = %s \n", buffer);
	int alt2 = alt1 ^ Seqnum;
	int2bin(alt2, buffer, BUF_SIZE - 1);
        printf("alt2 = %s \n", buffer);
	int alt3 = alt2 ^ Length;
	char * try = malloc(4*sizeof(char));
	sprintf(try,"%d",alt3);
	int test = atoi(try);
	int2bin(test, buffer, BUF_SIZE - 1);
        printf("alt3 = %s \n", buffer);
	return try;
	
        }	

	int getHeaderFromBit(const char * buff){//const achar * data, const size_t len, pkt_t * pkt
		char buffer[BUF_SIZE];
    		buffer[BUF_SIZE - 1] = '\0';
		const int data = atoi(buff);
		int2bin(data, buffer, BUF_SIZE - 1);
		
		//type
		int type = data >> 29;

		//Window
		int wi1= data << 3;
		int window = wi1 >> 27;

		//payload
		int pay1 = data << 8;
		int payload = pay1 >> 24;

		//crc
		int crc1 = data << 16;
		int crc = crc1 >> 16;

		return crc;
		
		
	}

	char *substring(const char *data, int start, int lenght){
		char *dest = malloc((lenght +1)*sizeof(char));
		int i=start;
		int it=0;		
		for(i;i<lenght;i++){
			dest[it] = data[i];
			it++;
		}
		dest[it+1]='\0';
		return dest;
	}

	

	void print_address(struct sockaddr_in6 *add){
	char lisible[100];
	void *ptr;
	ptr = &(add->sin6_addr);
	inet_ntop (AF_INET6, ptr, lisible, 100);
	
	printf("IPV6= %s\n",lisible);
}


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
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags=AI_CANONNAME;
	
	//récupération des informations
	int erreur = getaddrinfo(address, NULL, &hints, &res);
	if(erreur!=0){
		const char error[]="ERROR: getaddrinfo finished unsuccessfully";
		return error;
	}

	//création de l'adresse de retour
	struct sockaddr_in6 *add = (struct sockaddr_in6 *) res->ai_addr;
	
	//copie dans rval	
	memcpy((void *)rval,(void *)add,sizeof(*add));

	print_address(rval);
	
	return NULL;
	
}




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

	int main(int argc, const char *argv[]){
		struct sockaddr_in6 *retour=malloc(sizeof(struct sockaddr_in6));
		real_address(argv[1], retour);
		printf("TRY\n");
		int socket = create_socket(retour,15000,retour,12000);
		printf("OK! socket:%d portecoute:%d portenvoi:%d \n",socket, 13000, 9000);
		read_write_loop(socket);
		close(socket);
	}
