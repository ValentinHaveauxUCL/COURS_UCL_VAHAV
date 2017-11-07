#include "packet_interface.h"

/* Extra #includes */
#include <math.h>
#include <stdlib.h>
#include <zlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

//Comment tester un invalid padding?
//Comment tester qu'il n'y a pas d'header?
//Comment tester qu'il est unconsistant ?

struct __attribute__((__packed__)) pkt {
    //ordre a importance?? cmt assigner l'ordre et le nombre de bit?
    ptypes_t type : 3;
    uint8_t window : 5;
    uint8_t seqnum;
    uint16_t length;
    char *payload;
    uint32_t crc;
};


pkt_t* pkt_new()
{
	pkt_t *packet=(pkt_t *) malloc(sizeof(pkt_t));
	if(packet==NULL)
    {
        return NULL;
    }
    else
    {
        packet->payload=NULL;
        return packet;
    }
}

void pkt_del(pkt_t *pkt)
{
    free(pkt->payload);
    free(pkt);
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{


        uint16_t l=(data[2]<<8)|data[3];
        uint16_t reste=l%4;
        uint16_t pad;

        if(reste != 0)
            pad=4-reste;
        else
            pad=0;

    	if(l>MAX_PAYLOAD_SIZE || len>520)
    	{
        	pkt_del(pkt);
        	return E_LENGTH;
    	}
    	if(len==8)
    	{
        	pkt_del(pkt);
       	 	return E_NOPAYLOAD;
    	}
    	if(4+l+pad+4 != (uint16_t)len)
    	{
        	pkt_del(pkt);
        	return E_UNCONSISTENT;
    	}
    	else
    	{
        	pkt_set_length(pkt,l);
    	}


    	pkt_set_payload(pkt,data+4,l);
    	//uint32_t crc=(data[len-4]<<24)|(data[len-3]<<16)|(data[len-2]<<8)|(data[len-1]);
        uint32_t crc = (uint8_t) data[len-4];
        crc = (crc << 8) + (uint8_t) data[len-3];
        crc = (crc << 8) + (uint8_t) data[len-2];
        crc = (crc << 8) + (uint8_t) data[len-1];

    	uint32_t crc2=crc32(0,(Bytef *)data,len-4);
    	if(crc!=crc2)
    	{
        	pkt_del(pkt);
        	return E_CRC;
    	}
    	else
    	{
        	pkt_set_crc(pkt,crc);
    	}
    	pkt_set_seqnum(pkt,data[1]);
    //TYPE
    	ptypes_t type=data[0] >>5;
        if(type==PTYPE_DATA || type==PTYPE_ACK || type==PTYPE_NACK)
    	{
        	pkt_set_type(pkt,type);
    	}
        else
    	{
        	pkt_del(pkt);
        	return E_TYPE;
    	}
	//WINDOW

        uint8_t window=data[0]&0b00011111 ;
    pkt_status_code errorwin = pkt_set_window(pkt,window);
    if(errorwin!=PKT_OK) return errorwin;
    	return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    uint8_t window=pkt_get_window(pkt);
    uint16_t length=pkt_get_length(pkt);
    //uint32_t crc=pkt_get_crc(pkt);
    uint8_t seqnum = pkt_get_seqnum(pkt);
	uint8_t type =(uint8_t)pkt_get_type(pkt);
    uint16_t reste=length%4;
	uint16_t pad;
	if(reste!=0)
		pad=(4-reste)%4;
	else
		pad=0;
	if(length+4+4+reste > (uint16_t)*len){ return E_NOMEM;}
	else{
        type=type<<5;
        uint8_t byteone=type|window;
	buf[0]= byteone;
	buf[1]=seqnum;
	buf[2]=(length >>8)&0xFF;
	buf[3]=length&0xFF;
    uint16_t i;
	const char *payload =pkt_get_payload(pkt);
	for(i=0;i<length+pad;i++) {buf[i+4]=payload[i];}


        uint32_t crc = (uint32_t) crc32(0, (Bytef *) buf, 4 + length + pad);
        buf[i+4] = (crc >> 24) & 0xFF;
        buf[i+5] = (crc >> 16) & 0xFF;
        buf[i+6] = (crc >> 8) & 0xFF;
        buf[i+7] = crc & 0xFF;
	//uint32_t c2=htonl(crc);

        *len=4+length+pad+4;
        return PKT_OK;
	}


}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
        pkt->type=type;
        return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    if(window>MAX_WINDOW_SIZE)
    {
        return E_WINDOW;
    }
    else{
        pkt->window=window;
        return PKT_OK;}
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
        pkt->seqnum=seqnum;
        return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
        pkt->length=length;
        return PKT_OK;
}

pkt_status_code pkt_set_crc(pkt_t *pkt, const uint32_t crc)
{
        pkt->crc=crc;
        return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
							    const char *data,
								const uint16_t length)

{
    if(length > 512){ return E_LENGTH;}
    else{
      	if(pkt->payload != NULL) {free((char *)pkt->payload);}
	int pad;
	if (length%4 != 0)
		pad = 4- length%4;
	else
		pad =0;
      	int realSize= length + pad ;
	pkt_set_length(pkt, length);
      	pkt->payload=(char *) calloc(realSize,sizeof(char));
      	if (pkt->payload == NULL) {return E_NOMEM;}

      	memcpy(pkt->payload,(char *)data,length);
    	return PKT_OK;
    	}
}

ptypes_t pkt_get_type  (const pkt_t* pkt)
{
    return pkt->type;
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
    return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
    return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
    return pkt->length;
}

uint32_t pkt_get_crc   (const pkt_t* pkt)
{
    return pkt->crc;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    return pkt->payload;
}
