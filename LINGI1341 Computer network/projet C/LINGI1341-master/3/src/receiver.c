#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "create_socket.h"
#include "read_write_loop.h"
#include "real_address.h"
#include "wait_for_client.h"

#define MAXPKTSIZE 520
#define MAXSEQNUM 255

/*--------------------------------------------------------------------------------------------------*/

/*                            définition des variables globales				  														*/

/*--------------------------------------------------------------------------------------------------*/

char* file=NULL;
char* hostname;
int port=-1;
pkt_t *packet;

pkt_t * window[32];
int nProd = 0;
int oldN = 0;
int newN = 15;

/*pthread_mutex_t mutexP;
pthread_mutex_t mutexN;
pthread_mutex_init(&mutexP, NULL);
pthread_mutex_init(&mutexN, NULL);
int finish = 1;*/

/*--------------------------------------------------------------------------------------------------*/

/*                                   définition de fonctions																				*/

/*--------------------------------------------------------------------------------------------------*/

/*void* writer(){ //prend les paquets du buf pour creer new fichier
  i = 0;
  pkt_t temp;
  while (nProd > 0 || finish ==1){
    temp = window[i%32]; //Check si fini
    //write(temp);
    pthread_mutex_lock(&mutexP);
    nProd = nProd-1;
    pthread_mutex_unlock(&mutexP);
    pthread_mutex_lock(&mutexN);   
    oldN = oldN+1;
    newN = newN+1;
    pthread_mutex_unlock(&mutexN);
  }
}*/

/*void* listener(){ //regarde les paquets recus (met dans buffer si OK sinon envoie ACK)
  int bonus = 0;
  pkt_t temp2;
  while(finish == 1){
  //temp2 = rcv();
  if (temp2.crc == -1) { send(NACK);}
  
  pthread_mutex_lock(&mutexN);   
  if (temp2.seqnum == oldN){
    pthread_mutex_unlock(&mutexN);
    window[oldN%32]=temp2;
    pthread_mutex_lock(&mutexP);
    while (window[(oldN+1+bonus)%32]==NULL){
      bonus = bonus + 1;
    }
    nProd = nProd+1+bonus;
    bonus = 0;
    pthread_mutex_unlock(&mutexP);  
  }
  
  pthread_mutex_lock(&mutexN); 
  if (temp2.seqnum < oldN || temp2.seqnum =< newN){
    pthread_mutex_unlock(&mutexN);
    window[oldN%32]=temp2;  
  }
}*/

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
	return writed;
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
	return readed;
}

void receiver(int socket, char *filename)
{
	fd_set read, write;
	char readed[520];
	int f;
	if(filename==NULL)
		f=STDOUT_FILENO;
	else
		f=open(filename,O_WRONLY|O_CREAT|O_TRUNC);
	while(1){
	FD_ZERO(&read);
	FD_ZERO(&write);
	FD_SET(socket,&write);
	FD_SET(socket,&read);
	FD_SET(f,&write);
	if(select(socket+1,&read,&write, NULL,NULL)==-1)
	{
		fprintf(stderr,"Error selectreceiver\n");
		return;
	}

	if(FD_ISSET(socket,&write) && FD_ISSET(socket,&read))
	{
		printf("receiver readwritesocket\n");
		int rd=readSocket(socket,readed,520);
		if(rd>=0)
		{
			pkt_t *pkt=pkt_new();
			pkt_status_code errdec=pkt_decode((const char *)readed,(size_t)rd,pkt);
printf("lu : %s\n",pkt_get_payload(pkt));
			if(errdec==PKT_OK)
			{
				if(pkt_get_type(pkt)==PTYPE_DATA)
				{
					printf("On doit créer ACK/NACK\n");
				}
			}
			else
			{
				fprintf(stderr,"Error receiver decode\n");
				return;
			}
			pkt_del(pkt);
	printf("after del\n");
		}	
	}
	else if(FD_ISSET(f,&write))
	{
		printf("Write file\n");
		break;
	}
	}
}

int main(int argc, char **argv){

	hostname=malloc(50*sizeof(char));

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
	    		if(argc>=arg+2)
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
	int socket = create_socket(&addr,port,NULL,-1);
	if(socket==-1)
	{
		fprintf(stderr,"Create Socket error\n");
		close(socket);
		return -1;
	}
	int wait=wait_for_client(socket);
	if(wait==-1)
	{
		fprintf(stderr,"Impossible to connect after first reception\n");
		close(socket);
		return -1;
	}
	//read_write_loop(socket);
	receiver(socket,file);
	close(socket);

	return 0;

}
