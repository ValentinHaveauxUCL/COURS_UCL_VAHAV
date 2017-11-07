#include "create_socket.h"
#include <stdio.h>


int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {

    int sfd=socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP);

    if(sfd==-1)
    {
        perror("error creation socket in createsocket");
        return -1;
    }

    if(source_addr !=NULL)
    {
        if(src_port > 0 ) source_addr->sin6_port=htons(src_port);

        if(bind(sfd,(struct sockaddr *)source_addr,sizeof(struct sockaddr_in6))==-1)
        {
                perror("error bind in createsocket");
                return -1;
        }
    }

    if(dest_addr!=NULL)
    {
        if(dst_port>0) dest_addr->sin6_port=htons(dst_port);

        if(connect(sfd,(struct sockaddr *)dest_addr,sizeof(struct sockaddr_in6))==-1)
        {
            perror("error connect in createsocket");
            return -1;
        }
    }

    return sfd;
}
