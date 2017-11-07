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

/* Extra code */
/* Your code will be inserted here */
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

	int alt1 = Type ^ Window;
	int alt2 = alt1 ^ Seqnum;
	int alt3 = alt2 ^ Length;
	printf("HEADER1:%d\n",alt3);
	char * retour = malloc((HEADER_BYTES+1)*sizeof(char)); //(HEADER_BYTES+1)
	sprintf(retour,"%d",alt3);
	retour[HEADER_BYTES]='\0';
	return retour;
	
}


/* Create a new struct pkt, to be free'd by pkt_del
 * @return: An initialized struct pkt,
 * 			NULL if the system is out of memory */
pkt_t* pkt_new()
{
	pkt_t * newpkt = malloc(sizeof(pkt_t));
	//if(newpkt!=NULL){
	//	char *mydata = calloc(512,sizeof(char));
	//	newpkt->mydata = *mydata;
	//}
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
char *substring(const char *data, int start, int lenght){
		char *dest = malloc((lenght +1)*sizeof(char));
		int i;
		int it=0;		
		for(i=start;i<lenght;i++){
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
	char *header_string = substring(data,0,4*sizeof(char));
	int header_bytes = atoi(header_string);
	printf("HEADER: %s  int:%d\n",header_string,header_bytes);
	//prise de type
	int type = header_bytes >> ((HEADER_BYTES*8)-TYPE_BITS);
	printf("TYPE2=%d\n",type);
	if(type!=1 && type!= 2 && type!=4){
		return E_TYPE;
	}

	//prise de Window
	int wi1= header_bytes << TYPE_BITS;
	int window = wi1 >> ((HEADER_BYTES*8)-WINDOW_BITS);
	if(window<0 || window>31){
		//return E_WINDOW vu les pré, sais pas si nécessaire
	}
	//prise de seqnum
	int seq1 = header_bytes << (WINDOW_BITS+TYPE_BITS);
	int seqnum = seq1 >> ((HEADER_BYTES*8)-SEQNUM_BITS);
	if(seqnum<0 || seqnum>255){
		//return E_SEQNUM pareil
	}

	//prise de length
	int length1 = header_bytes << (WINDOW_BITS+TYPE_BITS+SEQNUM_BITS);
	int length = length1 >> ((HEADER_BYTES*8)-LENGTH_BITS);
	if(length!=(int)len){
		return E_LENGTH;
	}
	
	//free du buffer header
	free(header_string);

	//Prise du CRC
	char * CRC_string = substring(data, ((int) strlen(data))-CRC_BYTES,((int) strlen(data)));
	int crc = atoi(CRC_string);
	free(CRC_string);

	//Calcul et comparaison nouveau CRC
	int new_crc = (int) crc32(0, (const void*)data, len);
	if(new_crc != crc){
		return E_CRC;
	}

	//création du paquet
	const char * payload = substring(data, HEADER_BYTES+1,(int) strlen(data)-CRC_BYTES);
	pkt_set_type(pkt, (const ptypes_t) type);
	pkt_set_window(pkt, (const uint8_t) window);
	pkt_set_seqnum(pkt, (const uint8_t) seqnum);
	pkt_set_length(pkt, (const uint16_t) length);
	pkt_set_crc(pkt, (const uint32_t) crc);
	pkt_set_payload(pkt, payload, (const uint16_t) len); 
	
 	return PKT_OK;
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


int getAmountOfRealloc(const pkt_t *pkt){
	int res_mod = (pkt_get_length(pkt))%4;
	printf("[GETAMOUNTOFREALLOC] LEN=%d  RES_MOD=%d RET=%d\n",pkt_get_length(pkt),res_mod,4-res_mod);
	if(res_mod==0){
		return 0;
	} else {
		return 4-res_mod;
	} 
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
	if((int) *len<pkt_get_length(pkt)+amount+8){
		return E_NOMEM;
	} else {
		char * header = getHeaderToBits(pkt); //Création des 32bits de reader
		sprintf(buf,"%s",header);	//Ajout reader
		int i;
		int iterator;
		const char *data_packet = pkt_get_payload(pkt);
		printf("[IN ENCODE] data_packet=%s sizedata=%d\n",data_packet,pkt_get_length(pkt));
		for(i=4;i<pkt->mylength+HEADER_BYTES;i++){ //Ajout de data (payload)
			buf[i]=data_packet[i-4];
			printf("[IN ENCODE] posWritedata=%d   data=%s\n",i,buf);
		}
		for(iterator=0;iterator<amount;iterator=iterator+1){ //Padding
			printf("FOR   it=%d   amount=%d i+it=%d\n",iterator,amount,i+iterator); 
			sprintf(&buf[i+iterator],"%d",(int) 0); 
			printf("%s\n",buf);
		}
		printf("DATA_AFTER_PADDING=%s AMOUNT=%d\n",buf,amount );
		int crc = (int) crc32(0, (const void*)buf, i+iterator+1);
		printf("CRC1=%d\n",crc);
		sprintf(&buf[i+iterator],"%d",crc);
		*len = i + iterator+4;
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
	pkt->mywindow = window;
return PKT_OK;}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
	pkt->myseqnum = seqnum;
return PKT_OK;}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
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
	char *mydata = malloc((length+1)*sizeof(char));
	pkt->mydata = strcpy(mydata,data);
	mydata[length]=0;
	pkt_status_code erreur = pkt_set_length(pkt,length);
	if(erreur!=PKT_OK){return erreur;}
	return PKT_OK;
	
}

int main(int argc, const char * argv[]){
	char *test = malloc(6*sizeof(char));
	test = "salut";

	pkt_t *new = pkt_new();
	pkt_set_type(new,PTYPE_DATA);
	printf("TYPE1:%d\n",PTYPE_DATA);
	const uint8_t window= 10; 

	pkt_set_window(new, window);
	const uint8_t seqnum=10;

	pkt_set_seqnum(new,  seqnum);
	const uint16_t length = 5;

	pkt_set_length(new,length);
	const uint32_t crc= 2;

	pkt_set_crc(new, crc);
	pkt_set_payload(new,test,length);

	printf("[GET AFTER SET] PAY=%s\n",pkt_get_payload(new));

	char *buf = malloc(100*sizeof(char));	
	size_t len = 100*sizeof(char);
	pkt_encode(new, buf, &len);
	printf("[BUFF AFTeR ENCODE] BUF=%s\n",buf);

	pkt_t *recu = pkt_new();

	pkt_decode(buf, len, recu);
	printf("after decode\n");

	printf("%s\n",pkt_get_payload(recu));

	return 1;	
}

