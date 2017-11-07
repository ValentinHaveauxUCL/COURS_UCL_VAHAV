//
//  packet_implem.c
//  LINGI1341-Project1
//
//  Created by Jsday on 14/10/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h> //malloc
#include <zlib.h> //bytef
#include <string.h> // memcpy
#include <netinet/in.h> //htons
#include "packet_interface.h"

#define HEADER_SIZE 4
#define TIMESTAMP_SIZE 4
#define CRC_SIZE 4

struct __attribute__((__packed__)) pkt {
    ptypes_t type : 3; // 3bits
    uint8_t window : 5; // 5bits
    uint8_t seqnum; // 8bits
    uint16_t length; // 16bits
    uint32_t timestamp; // 32 bits
    char *payload; // max 4096bits
    uint32_t crc; // 32bits
};


/* Alloue et initialise une struct pkt
 * @return: NULL en cas d'erreur */
pkt_t* pkt_new()
{
    /* memory allocation */
    pkt_t *packet = (pkt_t *) malloc(sizeof(pkt_t));
    /* if memory allocation succeds */
    if(packet!=NULL)
    {
        packet->payload = NULL;
        return packet;
    }
    
    return NULL;
}

/* Libère le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associées
 */
void pkt_del(pkt_t *pkt)
{
    if (pkt != NULL) {
        /* free payload */
        if (pkt->payload != NULL) {
            free(pkt->payload);
        }
        /* free packet */
        free(pkt);
    }
}

/*
 * Décode des données reçues et crée une nouvelle structure pkt.
 * Le paquet reçu est en network byte-order.
 * La fonction vérifie que:
 * - Le CRC32 des données reçues est le même que celui décodé à la fin
 *   du flux de données
 * - Le type du paquet est valide
 * - La longeur du paquet est valide et cohérente avec le nombre d'octets
 *   reçus.
 *
 * @data: L'ensemble d'octets constituant le paquet reçu
 * @len: Le nombre de bytes reçus
 * @pkt: Une struct pkt valide
 * @post: pkt est la représentation du paquet reçu
 *
 * @return: Un code indiquant si l'opération a réussi ou représentant
 *         l'erreur rencontrée.
 */
pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
    /* type */
    ptypes_t type=data[0] >>5;
    if(type==PTYPE_DATA || type==PTYPE_ACK)
    {
        pkt_set_type(pkt,type);
    }
    else
    {
        fprintf(stderr,"(decode) type : %u\n", type);
        pkt_del(pkt);
        return E_TYPE;
    }
    
    /* window */
    uint8_t window=data[0]&0b00011111 ;
    pkt_status_code errorwin = pkt_set_window(pkt,window);
    if(errorwin!=PKT_OK) return errorwin;
    
    /* seqnum */
    pkt_set_seqnum(pkt,data[1]);
    
    /* get length */
    uint32_t length;
    
    /* extract header from raw packet */
    memcpy(&length, data+sizeof(uint16_t), sizeof(uint16_t));
    
    pkt->length = htons(length); // length
    if(pkt->length > MAX_PAYLOAD_SIZE || len>520)
    {
        fprintf(stderr,"(decode) length : %u\n", pkt->length);
        pkt_del(pkt);
        return E_LENGTH;
    }
    if(HEADER_SIZE+TIMESTAMP_SIZE+pkt->length+CRC_SIZE != (uint16_t)len)
    {
        fprintf(stderr,"(decode) unconsistent\n");
        pkt_del(pkt);
        return E_UNCONSISTENT;
    }
    else
    {
        pkt_set_length(pkt,pkt->length);
    }
    
    /* get timestamp */
    uint32_t timestamp;
    /* extract header from raw packet */
    memcpy(&timestamp, data+HEADER_SIZE, TIMESTAMP_SIZE);
    pkt->timestamp = timestamp;
    
    /* get payload */
    pkt->payload = malloc(sizeof(char)*pkt->length);
    if (pkt->payload == NULL) {
        fprintf(stderr, "(decode) payload allocation\n");
        return E_NOMEM;
    }
    memcpy(pkt->payload, data+HEADER_SIZE+TIMESTAMP_SIZE, pkt->length);
    
    /* get CRC */
    uint32_t crc;
    memcpy(&crc, data+HEADER_SIZE+TIMESTAMP_SIZE+pkt->length, CRC_SIZE);
    uint32_t byte_order_crc = htonl(crc);
    
    uint32_t crc2 = (uint32_t) (crc32(0,(Bytef *)data,(uInt)len - CRC_SIZE));
    
    
    if(byte_order_crc == crc2)
    {
        /* set CRC */
        pkt_set_crc(pkt,(byte_order_crc));
    }
    else
    {
        fprintf(stderr, "(decode) CRC");
        /* free packet */
        pkt_del(pkt);
        return E_CRC;
    }
    
    /* SUCCESS */
    return PKT_OK;
    
}

/*
 * Encode une struct pkt dans un buffer, prêt à être envoyé sur le réseau
 * (c-à-d en network byte-order), incluant le CRC32 du header et payload.
 *
 * @pkt: La structure à encoder
 * @buf: Le buffer dans lequel la structure sera encodée
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets écrit dans le buffer
 * @return: Un code indiquant si l'opération a réussi ou E_NOMEM si
 *         le buffer est trop petit.
 */
pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    fprintf(stderr, "encode\n");
    uint16_t payload_len = htons(pkt_get_length(pkt)); // get length
    uint8_t seqnum = pkt_get_seqnum(pkt);
    uint32_t timestamp = pkt_get_timestamp(pkt); // get timestamp
    
    fprintf(stderr, "payload_len : %zu and len : %zu and size : %zu", (size_t)payload_len, *len, (size_t)HEADER_SIZE+TIMESTAMP_SIZE+pkt_get_length(pkt)+CRC_SIZE);
    /* header + timestamp + payload + CRC ?> len */
    if(HEADER_SIZE+TIMESTAMP_SIZE+pkt_get_length(pkt)+CRC_SIZE > (uint16_t)*len){
        /* error buf to small */
        return E_NOMEM;
    }
    else{
        /* HEADER */
        //memcpy(buf, pkt, sizeof(uint8_t)); // type + window
        uint8_t type =(uint8_t)pkt_get_type(pkt);
        uint8_t window=pkt_get_window(pkt);
        type=type<<5;
        uint8_t byteone=type|window;
        buf[0]= byteone;
        memcpy(buf+sizeof(uint8_t), &seqnum, sizeof(uint8_t)); // seqnum
        memcpy(buf+sizeof(uint16_t), &payload_len, sizeof(uint16_t)); // length
        
        /* timestamp */
        memcpy(buf+HEADER_SIZE, &timestamp, TIMESTAMP_SIZE); // timestamp
        
        /* payload */
        memcpy(buf+HEADER_SIZE+TIMESTAMP_SIZE, pkt_get_payload(pkt), pkt_get_length(pkt));
        
        /* compute CRC */
        uint32_t crc = (uint32_t) crc32(0, (Bytef *) buf, HEADER_SIZE + TIMESTAMP_SIZE + pkt_get_length(pkt));
        uint32_t byte_order_crc = htonl(crc);
        
        memcpy(buf+HEADER_SIZE+TIMESTAMP_SIZE+pkt_get_length(pkt), &byte_order_crc, CRC_SIZE);
        
        *len = HEADER_SIZE+TIMESTAMP_SIZE+pkt_get_length(pkt)+CRC_SIZE;
        
        /* SUCCESS */
        return PKT_OK;
    }
    
}

/* Accesseurs pour les champs toujours présents du paquet.
 * Les valeurs renvoyées sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type  (const pkt_t* pkt)
{
    return pkt->type; // return type
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
    return pkt->window; // return window
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
    return pkt->seqnum; // return seqnum
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
    return pkt->length; // return length
}

uint32_t pkt_get_timestamp   (const pkt_t* pkt)
{
    return pkt->timestamp; // return timestamp
}

uint32_t pkt_get_crc   (const pkt_t* pkt)
{
    return pkt->crc; // return CRC
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    return pkt->payload; // return payload
}

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapté.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
    pkt->type = type; // set type
    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    if(window>MAX_WINDOW_SIZE)
    {
        return E_WINDOW;
    }
    else{
        pkt->window=window; // set window
        return PKT_OK;
    }
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    pkt->seqnum=seqnum; // set seqnum
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
    if (length > MAX_PAYLOAD_SIZE) {
        return E_LENGTH;
    }
    else{
        pkt->length=length; // set length
        return PKT_OK;
    }
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
    pkt->timestamp = timestamp; // set timestamp
    return PKT_OK;
}

pkt_status_code pkt_set_crc(pkt_t *pkt, const uint32_t crc)
{
    pkt->crc=crc; // set CRC
    return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length)
{
    if(length > MAX_PAYLOAD_SIZE){
        return E_LENGTH;
    }
    else{
        if(pkt->payload != NULL) {
            free((char *)pkt->payload);
        }
        
        pkt_set_length(pkt, length);
        pkt->payload=(char *) calloc(length,sizeof(char));
        if (pkt->payload == NULL) {
            /* not enough memory */
            return E_NOMEM;
        }
        
        memcpy(pkt->payload,(char *)data,length);
        return PKT_OK;
    }
}
