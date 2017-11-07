//
//  real_address.c
//  Chat
//
//  Created by Jsday on 28/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include "real_address.h"
#include <netdb.h> //getaddrinfo
#include <string.h> //memset, memcpy 

const char * real_address(const char * address, struct sockaddr_in6 * rval) {
    
    struct addrinfo hints;
    struct addrinfo *result;
    int a;
    
    memset(&hints, 0, sizeof(struct addrinfo)); //sets bytes to 0
    
    /*hint arguments*/
    hints.ai_family = AF_INET6; //allow IPv6 address
    hints.ai_socktype = SOCK_DGRAM; //datagram socket
    hints.ai_protocol = IPPROTO_UDP; //UDP protocol
    hints.ai_flags = AI_PASSIVE;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    a = getaddrinfo(address,NULL, &hints, &result);
    
    if(a != 0){//if not succeds
        return(gai_strerror(a));
    }
    
    struct sockaddr_in6 * addr = (struct sockaddr_in6 *) result->ai_addr;
    memcpy(rval, addr, result->ai_addrlen); // copy the resulting IPv6 address to rval
    
    freeaddrinfo(result);
    return NULL; //success
}
