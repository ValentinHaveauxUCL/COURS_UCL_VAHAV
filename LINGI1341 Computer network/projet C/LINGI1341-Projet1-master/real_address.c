#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

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
		char err[] = "ERROR: getaddrinfo finished unsuccessfully\n";
		const char *error = malloc(strlen(err)*sizeof(char));
		memcpy((void *) err, (void *)error, strlen(err));
		return error;
	}

	//création de l'adresse de retour
	struct sockaddr_in6 *add = (struct sockaddr_in6 *) res->ai_addr;
	
	//copie dans rval	
	memcpy((void *)rval,(void *)add,sizeof(*add));

	//print_address(rval);
	
	return NULL;
	
}


int main(int argc, const char *argv[]){
	if(argc!= 2){
		exit(1);
	}
	struct sockaddr_in6 *rval = malloc(sizeof(struct sockaddr_in6));

	printf("GO!\n");	

	real_address(argv[1],rval);

	
	return 0;
}



