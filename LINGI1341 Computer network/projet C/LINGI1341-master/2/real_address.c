#include "real_address.h"
#include <netdb.h>
#include <string.h>

const char * real_address(const char * address, struct sockaddr_in6 * rval) {

    struct addrinfo hints;
    struct addrinfo *result;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s=getaddrinfo(address,NULL, &hints,&result);
    if(s != 0)
        return(gai_strerror(s));

    struct sockaddr_in6 * addr = (struct sockaddr_in6 *) result->ai_addr;
    memcpy(rval, addr, result->ai_addrlen);

    freeaddrinfo(result);
	return NULL;
}
