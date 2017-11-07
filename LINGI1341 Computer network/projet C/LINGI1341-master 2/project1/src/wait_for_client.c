//
//  wait_for_client.c
//  Chat
//
//  Created by Jsday on 28/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include "wait_for_client.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define MAX_SEGMENT_SIZE 1024 //max size that can be sent
int wait_for_client(int sfd)
{
    
    char buffer[MAX_SEGMENT_SIZE];
    struct sockaddr_storage address;
    socklen_t address_len;
    ssize_t nread;
    //int serv;
    
    
    address_len = sizeof(struct sockaddr_storage);
    nread = recvfrom(sfd, buffer, MAX_SEGMENT_SIZE, 0,
                         (struct sockaddr *) &address, &address_len);
    if (nread == -1)
        return -1;
        
    /*char host[NI_MAXHOST], service[NI_MAXSERV];
        
    serv = getnameinfo((struct sockaddr *) &address,
                    address_len, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV);
    if (serv == 0){
        printf("From %s\n", host);
    }
    else{
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(serv));
    }
    
    char * resp = "your are connected ... \n";
    ssize_t resp_len = strlen(resp);
    if (sendto(sfd, resp, resp_len, 0,
                (struct sockaddr *) &address,
                   address_len) != resp_len)
        fprintf(stderr, "Error sending response\n");*/
    
    return 0;
}

