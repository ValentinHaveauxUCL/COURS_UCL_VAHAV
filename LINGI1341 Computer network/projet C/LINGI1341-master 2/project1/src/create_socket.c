//
//  create_socket.c
//  Chat
//
//  Created by Jsday on 28/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include "create_socket.h"
#include <stdio.h>
#include <netdb.h> //network address and service


int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {
    
    int sfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP); //init a socket
    
    if(sfd == -1)
    {
        fprintf(stderr, "cannot create a socket (create_socket)");
        return -1;
    }
    
    if(source_addr != NULL) //cannot be null
    {
        if(src_port > 0 ){
            source_addr->sin6_port=htons(src_port);
        }
			
        if(bind(sfd,(struct sockaddr *)source_addr,sizeof(struct sockaddr_in6)) == -1)
        {
            fprintf(stderr, "cannot bind (create_socket)");
            return -1;
        }
    }
		
    if(dest_addr!=NULL)//cannot be null
    {
        if(dst_port>0){
            dest_addr->sin6_port=htons(dst_port);
        }
			
        if(connect(sfd,(struct sockaddr *)dest_addr,sizeof(struct sockaddr_in6))==-1)
        {
            fprintf(stderr,"cannot connect (create_socket)");
            return -1;
        }
    }
		
    return sfd;//succeds
}
