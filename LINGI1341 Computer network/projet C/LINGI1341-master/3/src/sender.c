#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "packet_interface.h"
#include "real_address.h"
#include "read_write_loop.h"
#include "create_socket.h"
/*--------------------------------------------------------------------------------------------------*/
/*                            définition des variables globales				                              */
/*--------------------------------------------------------------------------------------------------*/
char* file=NULL;
char* hostname;
int port=-1;
char *buf;
int socketF;

pkt_t * window[32];
struct timeval timeBuf [32];

int nProd = 0; //nbr pkt produit non envoyé
int nSend = 16; //nbr pkt que l'on pourrait encore envoé
int newPKT = -1; //position du plus recend paquet envoyé
int oldPKT = 0; //position du plus vieux paquet envoyé

/*pthread_mutex_t mutexP;
pthread_mutex_t mutexS;
pthread_mutex_t mutexPkt;
pthread_mutex_init(&mutexP, NULL);
pthread_mutex_init(&mutexN, NULL);
pthread_mutex_init(&mutexPkt, NULL);
*/

/*--------------------------------------------------------------------------------------------------*/
/*                                   définition de fonctions																				*/
/*--------------------------------------------------------------------------------------------------*/

void* producer(); //lit le fichier et met les paquets dans le buf
void* sender2(); //envoie les paquets
void* listen2(); //ecoute les ACK (et renvoie si besoin)
void* checkTimer(); //verifie que le Timer le plus ancien ne depasse pas le RTT
void* fichierMode(); //lance tout les thread pour l'envoie de fichier
//int makePacketFromChar(bufRead, result);//Renvoie 0


/*void* fichierMode(){

	socketF = create_socket(NULL,0,&addr,port);
	if(socketF==-1)
	{
		fprintf(stderr,"Create Socket error\n");
		close(socket);
		//return -1;
	}
	//lancement des threads
	int err;
	pthread_t producer, sender, listener, timer;
	err=pthread_create(&producer, NULL, producer, NULL);
	err+=pthread_create(&sender, NULL, sender, NULL);
	err+=pthread_create(&listener, NULL, listener, NULL);
	err+=pthread_create(&timer, NULL, checkTimer, NULL);
		if(err!=0) {
			perror("pthread_create");
			return EXIT_FAILURE; }
	
	//fin des threads
	err=pthread_join(producer, NULL);
	err+=pthread_join(sender, NULL);
	err+= pthread_join(listener, NULL);
	err+= pthread_join(timer, NULL);
	if(err!=0) {
			perror("pthread_join");
			//return EXIT_FAILURE;
    }
	
	close(socketF);
}*/

/*void* producer(){
	int fd=0;
	char * bufRead;
	pkt_t temp = pkt_new();
	if (file!=NULL){
		fd=open(file, O_RDONLY);
		if (fd<0) {
			perror("file does not exist."); }
    }
	if(file==NULL) {
			perror("filename mais fd"); }
	int length = 1;
	int seqNumber = 0;
	
	while (length!=0){
		length = read(fd,(void*)bufRead, 512); //Renvoie 0 si il a rien lu
		if (length!=0) {
			//temp = makePacketFromChar(bufRead, lenght); // A faire
			while (window[seqNumber%32] != NULL) { // Fais rien car contient deja un packet pas envoyé
			}
			window[seqNumber%32]=temp;
			pthread_mutex_lock(&mutexP);
			nProd += 1;
			pthread_mutex_unlock(&mutexP);
			seqNumber += seqNumber;
			if (seqNumber == 256) { seqNumber = 0 ; }
		}
	}			
	close(fd);
}

void* sender(){
	int i = 0;
	while(true){ 
		if(nProd > 0 || nSend > 0 || window[i%32]==NULL ){
		//envoyé window[i%32] au receiver
		i++;
		//timeBuf[i%32] = getTime();
		pthread_mutex_lock(&mutexPkt);
		newPKT += 1;
		pthread_mutex_unlock(&mutexPkt);
		if (newPKT == 256) {newPKT = 0;}
		pthread_mutex_lock(&mutexP);
		pthread_mutex_lock(&mutexS);
		nProd -= 1;
		nSend -= 1;
		pthread_mutex_unlock(&mutexP);
		pthread_mutex_unlock(&mutexS);
		}
	}
}

void* listen2(){
	pkt_t temp2;
	int bonus;
	int numACK;
	int pos;
	
	while(true){
	//Attend un ack et le met dans temp2
	if(temp2.type == 4) { //NACK
	//Renvoie le paquet de window[temp2.seqnum%32]
	//Timer
	}
	else if (temp2.type == 2){ //ACK
		numACK = temp2.seqnum;
		if(numACK == oldPKT){ //si c'est le plus vieux paquet en buffer confirmé
		//Zone critique mutex
			timeBuf[numACK%32] = NULL;//Annuler le timer
			window[numACK%32]=NULL
		//Fin zone		
		//Checker le nombre de packet NULL à la suite et le mettre dans bonus
			pthread_mutex_lock(&mutexS);
			pthread_mutex_lock(&mutexPkt);
			nSend += 1 + bonus;
			oldPKT += 1 + bonus;
			pthread_mutex_unlock(&mutexS);
			pthread_mutex_unlock(&mutexPkt);
			if(oldPKT > 255) { oldPKT = oldPKT%32;}
		else {
			pthread_mutex_lock(&mutexPkt);
			pos = newPKT - oldPKT;
			pthread_mutex_unlock(&mutexPkt);
			if (pos>0){ // Si la fenetre d'envoie n'est pas reporté au debut
				if(numACK > oldPKT || numACK =< newPKT){ window[numACK]=NULL timeBuf[numACK] = NULL;} //Si l'ACK vient de la fenetre d'envoie on peux enlever le paquet
			}
			else{
				if(numACK > oldPKT || numACK > newPKT){ window[numACK]=NULL timeBuf[numACK] = NULL;} //Si la fenetre est decallé et que l'ACK est à la fin
				if(numACK < oldPKT || numACK =< newPKT){ window[numACK]=NULL timeBuf[numACK] = NULL;} //Si la fenetre est decallé et que l'ACK est au debut
			}
		}
	}
}

void* checkTimer(){	
	struct timeval temp;
	
	while(newPKT != -1){
		temp = getTime() - timeBuf[oldPKT]; //On check le timer du plus vieux paquet en permanance
		if( temp > 3){ // si le plus vieux timer depasse 3 sec
		//Zone Cririque
			//Renvoyé window[j]
			timeBuf[j] = getTime();
		//Fin Zone
		}
	}
}
*/
int writeSocket(int socket, int length,char *buffer)
{
	ssize_t writed=write(socket,(void *)buffer,length);
	if(writed==-1)
	{
		fprintf(stderr,"Error in writeSocket\n");
		return -1;
	}	
	if(writed==EOF)
		printf("EOFwrite\n");	
	return 0;
}

int readSocket(int socket, char *buffer, int length)
{
	ssize_t readed=read(socket,(void *)buffer,length);
	if(readed==-1)
	{
		fprintf(stderr,"Error in readSocket\n");
		return -1;
	}
	if(readed==EOF)
		printf("EOFread\n");		
	return 0;
}

int readFile(int f, char *buffer,int length)
{
	ssize_t readed=read(f,(void *)buffer,length);
	if(readed==-1)
	{
		fprintf(stderr,"Error in readFile\n");
		return -1;
	}
	if(readed==EOF)
		printf("EOFreadfile\n");
	return 0;
}
void sender(int socket,char *filename)
{
	//pkt_t *pkt=pkt_new();
	int f;
	printf("start sender\n");
	if(filename==NULL)
		f=STDIN_FILENO;
	else
		f=open(filename,O_RDONLY);
	fd_set read, write;
while(1){
	FD_ZERO(&read);
	FD_ZERO(&write);
	FD_SET(socket,&write);
	FD_SET(socket,&read);
	FD_SET(f,&read);
	char readed[512];
	if(select(socket+1,&read,&write,NULL,NULL)==-1)
	{
		fprintf(stderr,"Error selectsender\n");
		return;
	}
	if(FD_ISSET(socket,&read))
	{
		printf("Sender Read Socket\n");
		int rd=readSocket(socket,readed,4);//ACK OR NACK size ? 4 or 8
		if(rd==0)
		{
			pkt_t *pkt=pkt_new();
			pkt_status_code errdec=pkt_decode((const char *) readed,4,pkt);
			if(errdec==PKT_OK)
			{
				ptypes_t type=pkt_get_type(pkt);
				if(type==PTYPE_ACK)
				{
					printf("ACK\n");
				}

				else if(type==PTYPE_NACK)
				{
					printf("NACK\n");
				}
				else
				{
					fprintf(stderr,"Sender receive wrong type\n");
					return;
				}
			}
			pkt_del(pkt);		
		}
		
	}
	else if(FD_ISSET(socket,&write) && FD_ISSET(f,&read))
	{
		printf("Sender read file\n");
		int rf=readFile(f,readed,512);
		printf("hahaha %s\n",readed);
		if(rf==0)
		{
			pkt_t *pkt=pkt_new();
			pkt_set_type(pkt,PTYPE_DATA);
			pkt_set_length(pkt,rf);
			//win
			//seqnum

			pkt_status_code errenc=pkt_encode(pkt,buf,(size_t *)&rf);
			
			if(errenc==PKT_OK)
			{
				if(writeSocket(socket,rf+8,buf)==-1)
				{
					fprintf(stderr,"Error sender writesocket\n");
					return;
				}
			}
			else
			{
				fprintf(stderr,"Error sender encode\n");
				return;
			}
			
			pkt_del(pkt);
		}
	
	}

}
} 
//faut un while qqpart


int main(int argc, char **argv){

	hostname=malloc(50*sizeof(char));
	buf=malloc(520*sizeof(char));
	int i=0;
		while(i<32)
		{
			window[i]=NULL;
			i++;
		}
    	if(argc < 3)
    	{
     	fprintf(stderr, "Invalid number of argument : Need hostname AND port\n");
		free(hostname);
		return -1;

    	}
    	int arg;
    	for(arg=1;arg<argc;arg++)
    	{
        	if(strcmp("-f",argv[arg])==0 && argc>=arg+1)
        	{
            		file=argv[arg+1];
            		arg++;
            		printf("%s\n",file);
        	}
        	else if(strcmp("--filename",argv[arg])==0 && argc>=arg+1)
        	{
           		file=argv[arg+1];
            		arg++;
            		printf("%s\n",file);
        	}
        	else
        	{
            		strcpy(hostname,argv[arg]);
            		printf("%s\n", hostname);
	    		if(argc>=arg+1)
			{

            			if(atoi(argv[arg+1])!=0)
					port=atoi(argv[arg+1]);

            			else
				{
					fprintf(stderr, "Invalid port number\n");
					free(hostname);
					return -1;
				}
				printf("%d\n",port);
		    		arg++;
			}
	    		else
			{
				fprintf(stderr,"Need a port number or a hostname\n");
				free(hostname);
				return -1;
			}

        	}



    	}
    	if(arg==3 && port==-1)

    	{

        	fprintf(stderr, "Invalid number of argument : Need hostname AND port\n");
		free(hostname);
		return -1;

    	}

	

	struct sockaddr_in6 addr;
	//addr=malloc(sizeof(addr));
	const char *erraddr=real_address(hostname,&addr);
	if(erraddr!=NULL)
	{
		fprintf(stderr,"%s\n",erraddr);
		return -1;
	}
	int socket = create_socket(NULL,0,&addr,port);
	if(socket==-1)
	{
		fprintf(stderr,"Create Socket error\n");
		close(socket);
		return -1;
	}
	sender(socket,file);

	return 0;

}
