//
//  main.c
//  Database application
//
//  Created by Jsday on 21/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
//#include <netinet/in.h>

#include "record.h"

int main(int argc, const char * argv[]){
    
    struct record *r = malloc(sizeof(struct record));
    FILE *fp;
    
    fp = fopen("output", "r");
    //read record from file
    int read = record_read(r, fp);
    
    if (read < 0) {
        return EXIT_FAILURE;
    }
    
    printf("read result : %d\n", read);
    
    FILE *f;
    f = fopen("myoutput", "w");
    record_write(r, f);
    
    
    
    return 0;
}
