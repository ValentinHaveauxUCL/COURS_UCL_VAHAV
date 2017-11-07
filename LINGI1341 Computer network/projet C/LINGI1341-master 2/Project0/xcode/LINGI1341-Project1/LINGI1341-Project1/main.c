//
//  main.c
//  LINGI1341-Project1
//
//  Created by Jsday on 14/10/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "packet_interface.h"
#include <zlib.h>
#include <string.h>

/*
 struct __attribute__((__packed__)) pkt {
	ptypes_t type;//check part of the enum??
	uint8_t window; //check smaller than 32
	uint8_t seqnum; //check smaller than 255, mais normalement donc c'est bon
	uint16_t length;//plus petit que 512 is in BYTES
	uint32_t crc;
	char* data;//supposee allouee en un block
 };*/
void print_pkt(pkt_t* pack)
{
    printf("-----Packet:\n");
    printf("type  =%d\n",pkt_get_type(pack));
    printf("window=%hhu\n",pkt_get_window(pack));
    printf("seq   =%hhu\n",pkt_get_seqnum(pack));
    int length = pkt_get_length(pack);
    printf("length=%d\n",length);
    length = (length%4==0)? length: length+4-(length%4);
    int i;
    const char * data = pkt_get_payload(pack);
    if(data!=NULL){
        for(i=0;i<length;++i){
            if(i%4==0) printf("\n");
            printf("%3c;",data[i]);
        }printf("\n");
    }
    else printf("NULL\n");
    printf("crc   =%u\n",pkt_get_crc(pack));
}

int main()
{
    /* init pkt */
    pkt_t* pack = pkt_new();
    /* set type */
    if(pkt_set_type(pack,(ptypes_t)PTYPE_DATA)) printf("invalid command\n");
    /* set window */
    if(pkt_set_window(pack,10)) printf("invalid command\n");
    /* set seqnum */
    if(pkt_set_seqnum(pack,128)) printf("invalid command\n");
    /* set timestamp */
    if (pkt_set_timestamp(pack, (uint32_t)321)) printf("invalid command\n");
    /* set payload */
    char line[]= "12345678901";
    if(pkt_set_payload(pack,line,(uint16_t)11)) printf("invalid command\n");
    /* set length */
    pkt_set_length(pack,11);
    
    char test[16];
    //char *test2=(char *)test;
    uint8_t type=pkt_get_type(pack);
    type=type<<5;
    uint8_t window=pkt_get_window(pack);
    uint8_t byte=type|window;
    uint8_t seqnum=pkt_get_seqnum(pack);
    memcpy((test),&byte, 1);
    memcpy((test+1),&seqnum, 1);
    //*test=byte;
    //*(test+1)=pkt_get_seqnum(pack);
    uint16_t length=pkt_get_length(pack);
    memcpy((test+2),&length, 2);
    memcpy((test+4),pkt_get_payload(pack), 12);
    
    
    uint32_t crc= (uint32_t) crc32(0, (Bytef *)test,15);
    pkt_set_crc(pack,crc);
    printf("CRC = %u\n", pkt_get_crc(pack));
    
    char buff[30]; size_t len = 30;
    print_pkt(pack);
    if(pkt_encode(pack,buff,&len)) {printf("-----error encoding-----\n"); return -1;}
    print_pkt(pack);
    pkt_del(pack);
    
    pkt_t * pack2 = pkt_new();
    /* DECODING */
    if(pkt_decode(buff,len,pack2)) {
        printf("-----error decoding-----\n");
        return -1;
    }
    print_pkt(pack2);
    pkt_del(pack2);
    return 0;
}


