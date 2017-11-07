//
//  record.c
//  Database application
//
//  Created by Jsday on 21/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h> //uint
#include <netinet/in.h>
#include "record.h"

#define TYPE_SIZE 15
#define F_SIZE 1
#define LENGTH_SIZE 16
#define UUID_SIZE 32

/**
 * Initialise un enregistrement
 * @pre: r!= NULL
 * @post: record_get_type(r) == 0 && record_get_length(r) == 0
 *		 && record_has_footer(r) == 0
 * @return: 1 en cas d'erreur, 0 sinon
 */
int record_init(struct record *r)
{
    r->TYPE = 0; // set record type to 0
    r->LENGTH = 0; // set payload length to 0
    r->F = 0; // set F to 0
    
    return 0;
}

/**
 * Libère les ressources consommées par un enregistrement
 * @pre: r!= NULL
 */
void record_free(struct record *r)
{
    if (r->payload) {
        free(r->payload); // free payload
    }
    
    //free(r);
}

/**
 * Renvoie le type d'un enregistrement
 * @pre: r != NULL
 */
int record_get_type(const struct record *r)
{
    
    return r->TYPE;
}

/**
 * Définit le type d'un enregistrement
 * @pre: r != NULL
 * @post: record_get_type(r) == (type & 0x7FFF)
 */
void record_set_type(struct record *r, int type)
{
    r->TYPE = type;
}

/**
 * Renvoie la taille du payload de l'enregistrement
 * @pre: r!= NULL
 */
int record_get_length(const struct record *r)
{
    return r->LENGTH;
}

/**
 * Définit le payload de l'enregistrement, en copiant n octets
 * du buffer.
 * @pre: r != NULL && buf != NULL && n > 0
 * @post: record_get_length(r) == (n & 0xFFFF)
 && record_get_payload(<buf2>, n) == (n & 0xFFFF)
 *        && memcmp(buf, <buf2>, (n & 0xFFFF)) == 0
 * @return: -1 en cas d'erreur, 0 sinon
 */
int record_set_payload(struct record *r,
                       const char * buf, int n)
{
    r->payload = (char*)malloc(sizeof(char)*n);
    if (r->payload == NULL) {
        //could not allocate memory for footer
        return -1;
    }
    //copy buf into payload
    memcpy(r->payload, buf, n);
    r->LENGTH = n; // set length
    
    return 0;
}

/**
 * Copie jusqu'à n octets du payload dans un buffer
 * pré-alloué de taille n
 * @pre: r != NULL && buf != NULL && n > 0
 * @return: n', le nombre d'octets copiés dans le buffer
 * @post: n' <= n && n' <= record_get_length(r)
 */
int record_get_payload(const struct record *r,
                       char *buf, int n)
{
    //copy payload into buf
    memcpy(buf, r->payload, n);
    return record_get_length(r);
}

/**
 * Teste si l'enregistrement possède un footer
 * @pre: r != NULL
 * @return: 1 si l'enregistrement a un footer, 0 sinon
 */
int record_has_footer(const struct record *r)
{
    if (r!=NULL) {
        return r->F;
    }else{
        return -1;
    }
}

/**
 * Supprime le footer d'un enregistrement
 * @pre: r != NULL
 * @post: record_has_footer(r) == 0
 */
void record_delete_footer(struct record *r)
{
    if (r!=NULL) {
        memset(&r->UUID, 0, sizeof(uint32_t));
        r->F = 0; //set F to 0
    }
}

/**
 * Définit l'uuid d'un enregistrement
 * @pre: r != NULL
 * @post: record_has_footer(r) &&
 record_get_uuid(r, &<uuid2>) => uuid2 == uuid
 */
void record_set_uuid(struct record *r, unsigned int uuid)
{
    if(uuid!=0xFFFFFFFF){
        r->UUID = uuid;
        r->F = 1; // set F to 1
    }
}

/**
 * Extrait l'uuid d'un enregistrement
 * @pre: r != NULL
 * @post: (record_has_footer(r) && uuid != 0) ||
 *        (!record_has_footer(r) && uuid == 0)
 */
unsigned int record_get_uuid(const struct record *r)
{
    return r->UUID;
    
}

/**
 * Ecrit un enregistrement dans un fichier
 * @pre: r != NULL && f != NULL
 * @return: n', le nombre d'octets écrits dans le fichier.
 -1 en cas d'erreur
 */
int record_write(const struct record *r, FILE *f)
{
    size_t len = 0;
    len = fwrite(r, 1, sizeof(uint16_t), f);
    uint16_t LENGTH = htons(r->LENGTH);
    len = len + fwrite(&LENGTH, 1, sizeof(uint16_t), f);
    len = len + fwrite(r->payload, 1, r->LENGTH, f);
    
    if (r->F == 1) {
        uint32_t UUID = r->UUID;
        len = len + fwrite(&UUID, 1, sizeof(uint32_t), f);
    }
    
    return len;
}

/**
 * Lit un enregistrement depuis un fichier
 * @pre: r != NULL && f != NULL
 * @return: n', le nombre d'octets luts dans le fichier.
 -1 en cas d'erreur
 */
int record_read(struct record *r, FILE *f)
{
    size_t len = 0;
    
    uint32_t *header = malloc(sizeof(uint32_t));
    
    /* Read header*/
    int h = (int)fread(header, sizeof(uint32_t), 1, f);
    if (h != 0) {
        len = sizeof(uint32_t);
    }
        
    bzero(r, sizeof(struct record));
    
    //TYPE
    r->TYPE = *header;
    //F
    r->F = *header>>15;
    //LENGTH
    r->LENGTH = htons(*header>>16);
    
    /* Read payload*/
    r->payload = malloc(r->LENGTH);
    int l = (int)fread(r->payload, r->LENGTH, 1, f);
    if (l != 0) {
        len = len + r->LENGTH;
    }
    
    //get UUID if footer
    if (r->F == 1) {
        uint32_t *UUID = malloc(sizeof(uint32_t));
        /* Read header*/
        int u = (int)fread(UUID, sizeof(uint32_t), 1, f);
        if (u != 0) {
            len = len + sizeof(uint32_t);
        }
        r->UUID = *UUID;
        free(UUID);
    }
    
    free(header);
    
    fprintf(stderr, "len : %zu\n", len);
    
    return len;
}
