#include "packet_interface.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define HEADER_BYTES 4
#define TYPE_BITS 3
#define WINDOW_BITS 5
#define SEQNUM_BITS 8
#define LENGTH_BITS 16
#define CRC_BYTES 4

/* Extra #includes */
/* Your code will be inserted here */

struct __attribute__((__packed__)) pkt {
	ptypes_t mytype;
	uint8_t mywindow;
	uint8_t myseqnum;
	uint16_t mylength;
	uint32_t mycrc;
	char * mydata;
};

/*
 *Fonction qui permet d'obtenir un header sous forme d'une serie de bit a partir
 *d'une structure pkt_t
 * */
char * getHeaderToBits(const pkt_t *pkt){ 
	//type
	int value = (int) pkt->mytype;
	int Type = value << ((HEADER_BYTES*8)-TYPE_BITS);

	//Window
	int value_window = (int) pkt->mywindow;
	int Window = value_window << ((HEADER_BYTES*8)-(TYPE_BITS+WINDOW_BITS));

	//Seqnum
	int value_seqnum = (int) pkt->mywindow;
	int Seqnum = value_seqnum << ((HEADER_BYTES*8)-(TYPE_BITS+WINDOW_BITS+SEQNUM_BITS));

	//Length
	int value_length = (int) pkt->mylength;
	int Length = value_length;
	
	//Mise en commun des différentes parties du header
	int alt1 = Type ^ Window;
	int alt2 = alt1 ^ Seqnum;
	int alt3 = alt2 ^ Length;
	printf("HEADER: %d\n",alt3);
	//Copie du header sur le stack
	char * retour = malloc((HEADER_BYTES)*sizeof(char));
	memcpy((void *)retour, (void *)&alt3,HEADER_BYTES);
	return retour;
	
}


/* Create a new struct pkt, to be free'd by pkt_del
 * @return: An initialized struct pkt,
 * 			NULL if the system is out of memory */
pkt_t* pkt_new()
{
	pkt_t * newpkt = malloc(sizeof(pkt_t));
	return newpkt;
}

void pkt_del(pkt_t *pkt)
{
    	free(pkt->mydata);
	free(pkt);
}

/*
 *Retourne un pointeur vers un buffer comprenant
 *une partie de data (du start'eme element et de taille length)
 *
 */
char *substring(const char *data, int start, int length){
		char *dest = malloc((length +1)*sizeof(char));
		int i;
		int it=0;	
		//Copie byte par byte	
		for(i=start;i<length;i++){
			dest[it] = data[i];
			it++;
		}
		dest[it+1]='\0';
		return dest;
	}

/*
 * Decode data that has been received and create a new pkt_t from it.
 * This functions performs the following checks:
 * - Validates the CRC32 of the received data against the one present at the
 *   end of the data stream
 * - Validates the type of the packet
 * - Validates the length of the packet
 *
 * @data: The set of bytes that constitutes the packet
 * @len: The number of bytes received
 * @pkt: An allocated struct pkt, where the decoded data should be stored.
 *
 * @return: A status code indicating the success or the failure type.
 * 			Unless the error is E_NOHEADER, the packet has at least the
 * 			values of the header found in the data stream.
 */

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{	
	if(len<HEADER_BYTES){
		return E_NOHEADER;
	}
	
	//Prise des 32 premier bits de header 
	int *header_bytes = malloc(HEADER_BYTES*sizeof(char));
	memcpy((void *)header_bytes, (void *) data, HEADER_BYTES);
	
	//prise de type
	int type = *header_bytes >> ((HEADER_BYTES*8)-TYPE_BITS);
	if(type!=PTYPE_DATA && type!= PTYPE_ACK && type!=PTYPE_NACK){
		//return E_TYPE;
	}

	//prise de Window
	int wi1= *header_bytes << TYPE_BITS;
	int window = wi1 >> ((HEADER_BYTES*8)-WINDOW_BITS);
	if(window<0 || window>31){
		return E_WINDOW;
	}

	//prise de seqnum
	int seq1 = *header_bytes << (WINDOW_BITS+TYPE_BITS);
	int seqnum = seq1 >> ((HEADER_BYTES*8)-SEQNUM_BITS);
	if(seqnum<0 || seqnum>255){
		return E_SEQNUM;
	}

	//prise de length
	int length1 = *header_bytes << (WINDOW_BITS+TYPE_BITS+SEQNUM_BITS);
	int length = length1 >> ((HEADER_BYTES*8)-LENGTH_BITS);
	if(length!=(int)len-8 || length>512){
		return E_LENGTH;
	}

	//Prise du CRC
	int crc =0;
	memcpy((void *)&crc,(void *)&data[len-CRC_BYTES],CRC_BYTES);


	//Calcul et comparaison nouveau CRC
	int new_crc = (int) crc32(0, (const void*)data, len-CRC_BYTES);
	if(new_crc != crc){
		return E_CRC;
	}

	//création du paquet
	const char * payload = substring(data, HEADER_BYTES,len-CRC_BYTES);
	pkt_status_code erreur = PKT_OK;
	erreur = pkt_set_type(pkt, (const ptypes_t) type);
	erreur = pkt_set_window(pkt, (const uint8_t) window);
	erreur = pkt_set_seqnum(pkt, (const uint8_t) seqnum);
	erreur = pkt_set_length(pkt, (const uint16_t) length);
	erreur = pkt_set_crc(pkt, (const uint32_t) crc);
	erreur = pkt_set_payload(pkt, payload, (const uint16_t) len); 
	//Si au moins une fonction pkt_set_*** renvoie une erreur, la fonction pkt_decode renverra le dernier message d'erreur obtenu.
 	//return erreur;
	return erreur;
}

ptypes_t pkt_get_type(const pkt_t* pkt){
	return pkt->mytype;
}
uint8_t  pkt_get_window(const pkt_t* pkt){
	return pkt->mywindow;
}
uint8_t  pkt_get_seqnum(const pkt_t* pkt){
	return pkt->myseqnum;
}
uint16_t pkt_get_length(const pkt_t* pkt){
	return pkt->mylength;
}
uint32_t pkt_get_crc   (const pkt_t* pkt){
	return pkt->mycrc;
}


const char* pkt_get_payload(const pkt_t* pkt){
	return pkt->mydata;
};

/*
 *
 *
 *
 * */
int getAmountOfRealloc(const pkt_t *pkt){
	int res_mod = (pkt_get_length(pkt))%4;
	if(res_mod==0){
		return 0;
	} else {
		return 4-res_mod;
	} 
}


char * try(const pkt_t *pkt){
	uint8_t type = pkt_get_type(pkt) << 5;
	uint8_t window = pkt_get_window(pkt);
	uint8_t tw = type | window;
	uint8_t seqnum = pkt_get_seqnum(pkt);
	uint16_t length = pkt_get_length(pkt);
	char *test = malloc(sizeof(int));
	memcpy((void *)test, (void *)&tw,1);
	memcpy((void *)&test[1], (void *)&seqnum,1);
	memcpy((void *)&test[2], (void *)&length,2);
	int tryr = 0;
	memcpy((void *)&tryr,(void *)test,4);
	printf("VALUE:%d\n",tryr);
	return test;
}

/*
 * Convert a struct pkt into a set of bytes ready to be sent over the wires,
 * including the CRC32 of the header & payload of the packet
 *
 * @pkt: The struct that contains the info about the packet to send
 * @buf: A buffer to store the resulting set of bytes
 * @len: The number of bytes that can be written in buf.
 * @len-POST: The number of bytes written in the buffer by the function.
 * @return: A status code indicating the success or E_NOMEM if the buffer is
 * 		too small
 * */
pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{	
	int amount = getAmountOfRealloc(pkt);
	//Renvoie une erreur s'il n'y a pas assez de place dans le buffer
	if((int) *len<pkt_get_length(pkt)+amount+8){
		return E_NOMEM;
	} else {
		//get et ajout header
		char * header = try(pkt);
		memcpy((void *) buf,(void *)header,HEADER_BYTES);
		
		//get et ajout payload
		const char *data_packet = pkt_get_payload(pkt);
		if(data_packet==NULL){
			return E_NOPAYLOAD;
		}
		memcpy((void *)&buf[HEADER_BYTES],data_packet,pkt_get_length(pkt));

		//ajout padding;
		uint8_t null = 0;
		int i;
		for(i=0;i<amount;i++){
		memcpy((void *)&buf[HEADER_BYTES+pkt_get_length(pkt)],(void *)&null,sizeof(uint8_t));
		}

		//get et ajout crc32
		int crc = (int) crc32(0, (const void*)buf, HEADER_BYTES+pkt_get_length(pkt)+amount);
		memcpy((void *)&buf[HEADER_BYTES+pkt_get_length(pkt)+amount],(void *) &crc, CRC_BYTES);

		//mise à jour de len
		*len = HEADER_BYTES+pkt_get_length(pkt)+CRC_BYTES+amount;

		//libération donnée
		free(header);
		return PKT_OK;	
		
	}
	
}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
	if (type!=1&&type!=2&&type!=4){
		return E_TYPE;
	}
	pkt->mytype = type;
	
return PKT_OK;}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
	//if(window<0||window>31){
	//	return E_WINDOW;
	//}
	pkt->mywindow = window;
return PKT_OK;}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
	//if(seqnum<0||seqnum>255){
	//	return E_SEQNUM;
	//}
	pkt->myseqnum = seqnum;
return PKT_OK;}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
	//if(length<0||length>512){
	//	return E_LENGTH;
	//}
	pkt->mylength = length;
return PKT_OK;}

pkt_status_code pkt_set_crc(pkt_t *pkt, const uint32_t crc)
{
	pkt->mycrc = crc;
return PKT_OK;
	
}

pkt_status_code pkt_set_payload(pkt_t *pkt,const char *data, const uint16_t length)
{
	if(data==NULL){
		return E_NOPAYLOAD;	
	}
	//Copie des data sur le stack
	char *mydata = malloc((length+1)*sizeof(char));
	pkt->mydata = strcpy(mydata,data);
	mydata[length]=0;
	pkt_status_code erreur = pkt_set_length(pkt,length);
	return erreur;
	
}

int main(int argc, const char * argv[]){
	char *test = malloc(6*sizeof(char));
	test = "salut";

	pkt_t *new = pkt_new();
	pkt_set_type(new,PTYPE_DATA);

	const uint8_t window= 10; 

	pkt_set_window(new, window);
	const uint8_t seqnum=10;

	pkt_set_seqnum(new,  seqnum);
	const uint16_t length = 5;

	pkt_set_length(new,length);
	const uint32_t crc= 2;

	pkt_set_crc(new, crc);
	pkt_set_payload(new,test,length);
	printf("DATA sent = %s\n",pkt_get_payload(new));

	char *buf = malloc(100*sizeof(char));	
	size_t len = 100*sizeof(char);
	pkt_encode(new, buf, &len);
	pkt_t *recu = pkt_new();
	pkt_decode(buf, len, recu);


	printf("DATA recue = %s\n",pkt_get_payload(recu));

	return 1;	
}

