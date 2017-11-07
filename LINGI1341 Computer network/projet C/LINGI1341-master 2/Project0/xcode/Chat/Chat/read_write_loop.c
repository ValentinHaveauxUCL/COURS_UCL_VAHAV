//
//  read_write_loop.c
//  Chat
//
//  Created by Jsday on 28/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include "read_write_loop.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_SEGMENT_SIZE 1024 //max size that can be sent

void read_write_loop(int sfd) {
    
    fd_set sfds;
    char buff[MAX_SEGMENT_SIZE];
    FD_ZERO(&sfds);
    
    while(true)
    {
        FD_SET(STDIN_FILENO,&sfds);
        FD_SET(sfd,&sfds);
        
        select(sfd+1,&sfds,NULL,NULL,NULL);
        
        if(FD_ISSET(STDIN_FILENO,&sfds))
        {
            ssize_t r=read(STDIN_FILENO,buff,MAX_SEGMENT_SIZE);
            
            if(r == EOF){
                break;
            }
            
            int w = (int) write(sfd,buff,r);
            if(w == -1){
                fprintf(stderr, "error with write (read_write_loop)");
            }
            
        }else if(FD_ISSET(sfd,&sfds))
        {
            ssize_t r=read(sfd,buff,MAX_SEGMENT_SIZE);
            
            if(r == EOF){
                break;
            }
            
            int w = (int) write(STDOUT_FILENO,buff,r);
            
            if(w == -1){
                fprintf(stderr, "error with write (read_write_loop)");
            }
        }
    }
}
