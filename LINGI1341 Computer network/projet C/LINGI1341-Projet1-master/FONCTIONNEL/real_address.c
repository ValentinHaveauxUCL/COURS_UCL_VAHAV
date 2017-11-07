#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

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
	//INUTILE NON? PAS FAIRE 2 LIGNE EN UNE?
	//copie dans rval	
	memcpy((void *)rval,(void *)add,sizeof(*add));

	print_address(rval);
	//ATTENTION VALEUR DE RETOUR
	return NULL;
	
}

