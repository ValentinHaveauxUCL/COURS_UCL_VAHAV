#include "socket.h"
#include "packet.h"
#define TIME 5
#define WINDOW_NBR 5
#define MAXSEQNUM 255
#define PACKET_MAX_SIZE 520
	char *hostname;
	int port;
	int check_file;
	const char *filename;
	int backup_STDOUT;
	char *packet_buffer[PACKET_MAX_SIZE];
	pkt_t *buffer_p[WINDOW_NBR];

void init(){
	hostname = malloc(40*sizeof(char));
	strcpy(hostname, "::");
	port = 0;
	check_file = 0;
	filename = NULL;
	backup_STDOUT=0;
}

void sendACK(int fd, int nseq){
	pkt_t *ACK = pkt_new();
	ACK->myseqnum=nseq;
	ACK->mytype = PTYPE_ACK;
	char pa[520];
	int size = 520;
	pkt_encode(ACK, pa,(size_t *) &size);
	write(fd, (void *) pa, size);
}

void sendNACK(int fd, int nseq){
	pkt_t *ACK = pkt_new();
	ACK->myseqnum=nseq+1;
	ACK->mytype = PTYPE_NACK;
	char pa[520];
	int size = 520;
	pkt_encode(ACK, pa,(size_t *) &size);
	write(fd, (void *) pa, size);
}

int check_for_seq_buffer(int seqnum, int a, int b){
	if(a==b){
		return FALSE;
	}
	int i;
	int retour=FALSE;
	for(i=a;i<b;i++){
		if((buffer_p[i])->myseqnum==seqnum){
			retour=1;
		}
	}
	return retour;
}

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
	int length;
	int readed =FALSE;
	while(boolean){
		FD_ZERO(lecture);
		FD_SET(fd_socket, lecture);
		timer->tv_sec=TIME;

		int timer_check = select(fd_socket+1,lecture,NULL,NULL,timer);
		if(timer_check>0){
			pkt_t *pkt = pkt_new();
			if(FD_ISSET(fd_socket, lecture)){
				//recuperation de la longueur lue
				length = read(fd_socket, (void *) packet_buffer, PACKET_MAX_SIZE);
				printf("LEN: %d\n", length);
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
					if(numseq<=MAX_seq_w && numseq>=MIN_seq_w && !check_for_seq_buffer(numseq,nseq_pos, nseq_offset)){
						//si celui attendu
						if(numseq==nseq_wait){
							//si longueur est pas nulle, continue
							if(pkt->mylength!=0){
								//Ecriture
								//write(STDOUT_FILENO, (void *) pkt->mydata, pkt->mylength);
								printf("DATA: %s\n",pkt->mydata);
								//Si suivant present dans buffer, ajout
								int tmp = numseq;
								while(check_for_seq_buffer(tmp+1,nseq_pos, nseq_offset)){
									int posi = get_next_buffer(numseq,nseq_pos, nseq_offset);
									write(STDOUT_FILENO, (void *) buffer_p[posi]->mydata, pkt->mylength);
									tmp++;
									nseq_pos++;
								}	
								//Si tout les seqnum correctement reçu jusque ici, envoi ACK
								if(numseq==ACK_OK){
									sendACK(fd_socket, numseq);
									ACK_OK++;
									if(ACK_OK>MAXSEQNUM){
										ACK_OK=0;
									}
								}
								nseq_wait++;
								if(nseq_wait>MAXSEQNUM){
									nseq_wait=0;
								}
								if(numseq==MIN_seq_w){
									MIN_seq_w++;
									MAX_seq_w++;
								}
							//si longueur est nulle, fin
							} else {
								boolean = FALSE;
							}
						//si pas celui attendu
						} else {
							sendNACK(fd_socket, numseq);
							buffer_p[nseq_offset]=pkt;
							nseq_offset++;
							if(nseq_offset>WINDOW_NBR && nseq_pos!=0){
								nseq_offset=0;
							}
						}
					//Si déjà reçu, nie et envoi nack pour desire
					} else if(check_for_seq_buffer(numseq, nseq_pos, nseq_offset)) {
						sendNACK(fd_socket,nseq_wait);
					}
				} else {
					const char *r = "Error: a problem occured during 'decode()'\n";
					write(STDERR_FILENO, (void *) r, strlen(r));
				}
			}
		} else if(timer_check==0){
			//delais timer depasse.
			//const char* err = "Error: timer out of time, no data received\n";
			//write(STDERR_FILENO, err, strlen(err));
			//boolean = FALSE;
			//break;
		}
		
	}
}

int main(int argc, const char *argv[]){
	//initialisation	
	init();
	
	//recherche des différentes données sur les paramètres
	int i;
	for(i=1;i<argc;i++){
		if(strcmp("-f",argv[i])==0 && argc>=i+1){
			filename = argv[i+1];
			i++;
		}
		else if(strcmp("--filename",argv[i])==0 && argc>=i+1){
			filename = argv[i+1];
			check_file = 1;
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
	//int fd_f = open("out.txt",O_RDWR);
	//int back = dup(STDOUT_FILENO);
	//int try = dup2(fd_f, STDOUT_FILENO);
	
	//Attente
	int wait = wait_for_client(socket_server);
	receive_data_transfered(socket_server);

	//Backup /
	//dup2(back, STDOUT_FILENO);
	//close(fd_f);
	
	
	return 0;
}
