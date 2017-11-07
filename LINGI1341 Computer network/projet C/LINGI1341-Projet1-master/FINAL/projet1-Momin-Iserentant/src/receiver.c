#include "socket.h"
#include "packet.h"
#include <sys/stat.h>
#define TIME 5
#define WINDOW_NBR 5
#define MAXSEQNUM 255
#define PACKET_MAX_SIZE 520
	//création des variables
	char *hostname; //adresse locale
	int port; //port d'écoute
	int check_file; //présence d'un paramètre de type fichier
	const char *filename; //nom du paramètre de type fichier
	pkt_t *buffer_p[WINDOW_NBR]; //buffer des packets reçus non séquencés
	int fd_filer; //filedescriptor de la redirection de STDOUT
	char * packet_buffer; //buffer de lecture des packets du socket.

//initialisation des variables
void init(){
	fd_filer=0;
	hostname = malloc(40*sizeof(char));
	strcpy(hostname, "::");
	port = 0;
	check_file = 0;
	filename = NULL;
	packet_buffer = malloc(PACKET_MAX_SIZE*sizeof(char));
	int i;
	for(i=0;i<WINDOW_NBR;i++){
		buffer_p[i]=NULL;
	}
}

//libération des ressources
void free_r(){
	free(hostname);
	free(packet_buffer);
}

//envoi d'un packet d'acquis
void sendACK(int fd, int nseq){
	char *ri = "";
	pkt_t *ACK = pkt_new();
	(*ACK).myseqnum=nseq+1;
	(*ACK).mytype = PTYPE_ACK;
	(*ACK).mywindow=1;
	ACK->mydata=ri;
	(*ACK).mylength=0;
	char pa[520];
	int size = 520;
	pkt_encode(ACK, pa,(size_t *) &size);
	write(fd, (void *) pa, size);

}

//envoi d'un packet de non-acquis
void sendNACK(int fd, int nseq){
	char *ri = "";
	pkt_t *NACK = pkt_new();
	(*NACK).myseqnum=nseq;
	(*NACK).mytype = PTYPE_NACK;
	(*NACK).mywindow=1;
	NACK->mydata=ri;
	(*NACK).mylength=0;
	char pa[520];
	int size = 520;
	pkt_encode(NACK, pa,(size_t *) &size);
	write(fd, (void *) pa, (size_t) size);

}

//Vérification de la présence d'un packet avec le num de séquence 'seqnum' dans buffer_p
int check_for_seq_buffer(int seqnum, int a, int b){
	if(a==b){
		if(buffer_p[a]!=NULL && (*(buffer_p[a])).myseqnum==seqnum){
			return TRUE;
		}
	}
	int i;
	int retour=FALSE;
	for(i=a;i<b;i++){
		if((*(buffer_p[a])).myseqnum==seqnum){
			retour=TRUE;
		}
	}
	return retour;
}

//retourne le packet dont le numéro de séquence est le suivant de 'seqnum' si il est dans buffer_p
int get_next_buffer(int seqnum, int a, int b){
	int i;
	int retour=FALSE;
	for(i=a;i<b;i++){
		if((buffer_p[i])->myseqnum==seqnum+1){
			retour=i;
		}
	}
	return retour;
}

//Vérification de la présence du packet dont le numéro de séquence est 'seqnum' dans la fenêtre
int is_in_window(int seqnum, int max, int min){
	if(max>=min){
		return (seqnum<= max && seqnum >=min);
	} else {
		return (seqnum<=max && seqnum>=min);
	}
}

//réception des packets
void receive_data_transfered(int fd_socket){
	//Structure de timer
	struct timeval *timer = malloc(sizeof(struct timeval));
	timer->tv_sec=TIME;
	timer->tv_usec=0;
	
	//Assignation au set de descripteurs
	fd_set *lecture;
	lecture = malloc(sizeof(*lecture));
	FD_ZERO(lecture);
	FD_SET(fd_socket, lecture);	

	//Lecture sur le socket
	int MIN_seq_w=0;
	int MAX_seq_w=MIN_seq_w+WINDOW_NBR;
	int nseq_wait=MIN_seq_w;
	int nseq_offset=MIN_seq_w;
	int nseq_pos=MIN_seq_w;
	int ACK_OK=MIN_seq_w;
	int boolean = TRUE;
	int readed =FALSE;
	int length;
	while(boolean){
		pkt_t *pkt = pkt_new();
		FD_ZERO(lecture);
		FD_SET(fd_socket, lecture);
		timer->tv_sec=TIME;
		int timer_check = select(fd_socket+1,lecture,NULL,NULL,timer);
		if(timer_check>0){
			if(FD_ISSET(fd_socket, lecture)){
				//recuperation de la longueur lue
				length = read(fd_socket, (void *) packet_buffer, PACKET_MAX_SIZE);
				timer_check=-1;
				readed=FALSE;
				if(length!=-1){
					readed=TRUE;
				}
				
			} 
			if(readed){
				//decodage du packet
				pkt_status_code retour = pkt_decode((const char *) packet_buffer, (const size_t) length, pkt);
				//si pas erreurs, suite
				if(retour==PKT_OK){
					//prise du numéro de seq
					int numseq = pkt->myseqnum;
					//si pas compris dans la fenêtre et déjà reçu, nie
					if( is_in_window(numseq,MAX_seq_w,MIN_seq_w) && !check_for_seq_buffer(numseq,nseq_pos, nseq_offset)){
						//si celui attendu
						if(numseq==nseq_wait){
							//si longueur est pas nulle, continue
							if((*pkt).mylength!=0){
								//Ecriture
								write(fd_filer, (void *) pkt->mydata, pkt->mylength);
								//Si suivant present dans buffer, ajout
								if(numseq==MIN_seq_w){
									MIN_seq_w++;
									MAX_seq_w++;
									if(MIN_seq_w>MAXSEQNUM){
									MIN_seq_w=0;
									}
									if(MAX_seq_w>MAXSEQNUM){
									MAX_seq_w=0;
									}
								}
								int tmp = numseq;
								//Check de la présence d'un packet dans buffer_p qui suit celui reçu.
								while(check_for_seq_buffer(tmp+1,nseq_pos, nseq_offset)){
									int posi = get_next_buffer(tmp,nseq_pos, nseq_offset);
									write(fd_filer, (void *) buffer_p[posi]->mydata, buffer_p[posi]->mylength);
									
									tmp++;
									nseq_pos++;
									nseq_wait++;
									if(nseq_wait>MAXSEQNUM){
									nseq_wait=0;
									}
									if(nseq_pos>WINDOW_NBR){
									nseq_pos=0;
									}
									ACK_OK++;
									if(tmp==MIN_seq_w){
									MIN_seq_w++;
									MAX_seq_w++;
									if(MIN_seq_w>MAXSEQNUM){
									MIN_seq_w=0;
									}
									if(MAX_seq_w>MAXSEQNUM){
									MAX_seq_w=0;
									}
								}
								}	
								//Si tout les seqnums correctement reçu jusque ici, envoi ACK
								if(tmp==ACK_OK){
									sendACK(fd_socket, tmp);
									ACK_OK++;
									if(ACK_OK>MAXSEQNUM){
										ACK_OK=0;
									}
								}
								nseq_wait++;
								if(nseq_wait>MAXSEQNUM){
									nseq_wait=0;
								}
							//si longueur est nulle, fin
							} else {
								sendACK(fd_socket,numseq);
								boolean = FALSE;
							}
						//si pas celui attendu
						} else {
							sendNACK(fd_socket, nseq_wait);
							if(!check_for_seq_buffer(numseq,nseq_pos, nseq_offset)){
							buffer_p[nseq_offset]=pkt;
							nseq_offset++;	
							}	
							if(nseq_offset>WINDOW_NBR && nseq_pos!=0){
								nseq_offset=0;
							}
						}
					//Si déjà reçu, nie et envoi nack pour désiré
					} else {
						if(!check_for_seq_buffer(numseq,nseq_pos, nseq_offset) && is_in_window(numseq,MAX_seq_w,MIN_seq_w)){
						 //ne rien faire
						} else {
						sendNACK(fd_socket, nseq_wait);
						}
					}
				//erreur lors de decode
				} else {
					sendNACK(fd_socket, nseq_wait);
					const char *r = "Error: a problem occured during 'decode()'\n";
					write(STDERR_FILENO, (void *) r, strlen(r));
				}
			readed=FALSE;
			}
		}

		
	}
	
	//free
	free(timer);
	free(lecture);
}

int main(int argc, const char *argv[]){
	//initialisation	
	init();
	
	//recherche des différentes données sur les paramètres
	int i;
	for(i=1;i<argc;i++){
		if(strcmp("-f",argv[i])==0 && argc>=i+1){
			filename = argv[i+1];
			check_file=TRUE;
			i++;
		}
		else if(strcmp("--filename",argv[i])==0 && argc>=i+1){
			filename = argv[i+1];
			check_file = TRUE;
			i++;
		}
		else{
			int test_port = atoi(argv[i]);
			if(test_port!=0){
				port = test_port;
			} else {
				strcpy(hostname, argv[i]);
			}
		}
	}

	//Vérification des paramètres
	if(port==0){
		const char *err = "Error: Invalid port number parameter! Please enter a valid port number.";
		write(STDERR_FILENO, err, strlen(err));
	}

	//Récupération de l'adresse réelle IPv6
	struct sockaddr_in6 *serv_addr;
	serv_addr = malloc(sizeof(*serv_addr));
	const char *ret = real_address(hostname, serv_addr);
	if(ret!=NULL){
		write(STDERR_FILENO, ret, strlen(ret));
	}

	//Création du socket
	int socket_server = create_socket(serv_addr, port, NULL,0);
	if(socket_server==-1){
		exit(-1);
	}


	//Redirection de stdout
	int back;
	int fd_f;
	if(check_file==TRUE){
	fd_f = open(filename,O_RDWR|O_CREAT, NULL);
	chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
	back = dup(STDOUT_FILENO);
	fd_filer = dup2(fd_f, STDOUT_FILENO);
	}
	
	//Attente
	
	wait_for_client(socket_server);

	receive_data_transfered(socket_server);

	//Backup
	if(check_file==1){
	dup2(back, STDOUT_FILENO);
	close(fd_f);
	}
	close(socket_server);	

	//free
	free_r();
	free(serv_addr);
	
	
	return 0;
}
