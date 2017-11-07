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

int wait_for_client(int sfd)
{
    
    char buffer[520];
    struct sockaddr_in6 *address = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
    socklen_t address_len = sizeof(*address);
    int rcv,con;
    
    rcv = (int)recvfrom(sfd,buffer,520,MSG_PEEK,(struct sockaddr *)address,&address_len);
    con = connect(sfd,(struct sockaddr *)address,address_len);
    
    if(rcv == -1 || con == -1)
        return -1;
    
    return 0;
}

