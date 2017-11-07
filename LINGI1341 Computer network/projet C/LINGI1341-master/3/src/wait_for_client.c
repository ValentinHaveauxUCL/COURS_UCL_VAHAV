#include "wait_for_client.h"
#include <netinet/in.h>
#include <stdlib.h>

int wait_for_client(int sfd)
{

	char buffer[520];
    struct sockaddr_in6 *address=(struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
    socklen_t addressL=sizeof(*address);
    int r,c;
    r=recvfrom(sfd,buffer,520,MSG_PEEK,(struct sockaddr *)address,&addressL);
    c=connect(sfd,(struct sockaddr *)address,addressL);
    if(r==-1 || c==-1)
        return -1;

    return 0;
}



