

#include "socket.h"
#include "packet.h"
#define TIME 1
#define TIME_s 1
#define TIME_m 150000
#define WINDOW_NBR 5
#define PACKET_MAX_SIZE 520
#define PAYLOAD_MAX_SIZE 512
#define MAXSEQNUM 255

typedef struct Time{
	int sec;
	int micro;
} Time;

typedef struct Timer{
	struct timeval *timer_t;
	int seqnum;
} Timer;
	Timer *timer_buffer[WINDOW_NBR]; //buffer des timers
	char *hostname; //adresse du receiver
	int port; //port du receiver
	int check_file; //présence d'un paramètre de type fichier
	const char *filename; //nom du paramètre de type fichier
	char *packet_buffer; //buffer de la lecture sur le socket
	pkt_t *window[WINDOW_NBR]; //buffer des packets envoyés
	int dernierenvoi; //numéro de séquence du dernier packet envoyé
	int fd_filer; //filedescriptor de STDIN redirigé.
	int index_offset; //index de l'écriture dans window
	int checking; //nombre d'élement de packet dans window
	int send_count; //nombre de packet envoyé sans ACK
	int end_start; //condition de sortie du while
	int last_timer_setup; //position dans time_buffer du dernier timer ayant dépassé le délai
	int sum_readed;//nombre de bytes déjà lu dans le fichier
	int file_length;//longueur en bytes du fichier
	int rec_win; //window du receveur;
	
//retourne la différence de temps entre les deux structure
Time getTime(struct timeval * start, struct timeval * stop){
	int dMicro = (*stop).tv_usec - (*start).tv_usec;
	int dSec = (*stop).tv_sec - (*start).tv_sec;
	int sec;
	int micro;
	if(dMicro<0){
		sec = dSec -1;
		micro = 1000000+dMicro;
	} else {
		sec = dSec;
		micro = dMicro;
	}
	Time retour = {sec,micro}; 
	return retour;
	
}

//initialisation des variables
void init(){
	rec_win = WINDOW_NBR;
	end_start = TRUE;
	file_length=0;
	sum_readed =0;
	last_timer_setup = -1;
	fd_filer=0;
	fd_filer=0;
	index_offset = 0;
	checking =0;
	send_count = 0;
	hostname = malloc(40*sizeof(char));
	packet_buffer = malloc(PACKET_MAX_SIZE*sizeof(char));
	strcpy(hostname, "::");
	port = 0;
	check_file = 0;
	filename = NULL;
	dernierenvoi=-1;
}

//Vérifie si un timer actif est dépassé
int check_for_timer(){
	int retour=-1;
	int i;
	struct timeval *now = malloc(sizeof(struct timeval));
	gettimeofday(now, NULL);
	for(i=0;i<WINDOW_NBR;i++){
		if(timer_buffer[i]!=NULL){
			Time div = getTime((timer_buffer[i])->timer_t, now);
			if(div.micro>=TIME_s){
				last_timer_setup = i;
				return (*(timer_buffer[i])).seqnum;
			}
		}
	}
	free(now);
	return -1;
}

//libère la mémoire.
void free_r(){
	free(hostname);
	free(packet_buffer);
}

//Vérifie la présence d'un packet ayant seqnum en numéro de séquence dans le buffer
int look_for_seq_buffer(int seqnum, int a, int b){
	int retour=-1;	
	if(a==b){
		if(window[a]!= NULL && (*(window[a])).myseqnum==seqnum){
			return a;
		}
	} else {
	int i;
	for(i=a;i<b;i++){
		if(window[i]!= NULL && (*(window[i])).myseqnum==seqnum){
			retour=i;
		}
	}
	}
	return retour;
}

//ACK un packet et libère les ressources associés
void ack_to_seq_buffer(int seqnum, int a, int b){
	if(a==b){
		if(window[a]!= NULL && (*(window[a])).myseqnum<=seqnum){
			free(window[a]->mydata);
			free(window[a]);
			free(timer_buffer[a]->timer_t);
			free(timer_buffer[a]);
			window[a]=NULL;
			timer_buffer[a]=NULL;
			checking--;
			send_count--;
		}
	} else {
	int i;
	for(i=a;i<b;i++){
		if(window[i]!= NULL && (*(window[i])).myseqnum<=seqnum){
			free(window[i]->mydata);
			free(window[i]);
			free(timer_buffer[i]->timer_t);
			free(timer_buffer[i]);
			window[i]=NULL;
			timer_buffer[i]=NULL;
			checking--;
			send_count--;
		}
	}
	}
}

//Envoi le fichier
void send_data(int fd_socket){
	//Structure de timer pour le select
	struct timeval *timer = malloc(sizeof(struct timeval));
	timer->tv_sec=TIME;
	timer->tv_usec=0;
	
	//Assignation au set de descripteurs
	fd_set *lecture;
	lecture = malloc(sizeof(*lecture));
	FD_ZERO(lecture);
	FD_SET(fd_socket, lecture);
	

	int MIN_w = 0;	//borne inférieur window
	int MAX_w = MIN_w + WINDOW_NBR; //borne max window
	int boolean = TRUE;
	int length;
	while(boolean){
		//vérification des timers
		int seq_po = check_for_timer();
		if(seq_po>=0){
			//réenvoi si dépassé
			int index_tmp = look_for_seq_buffer(seq_po, 0, WINDOW_NBR);
			if(window[index_tmp]!=NULL){//Corruption du num de séquence dans l'acquis sinon
			char *tabl = malloc(PACKET_MAX_SIZE*sizeof(char));
			size_t mylong=(size_t) PACKET_MAX_SIZE;
			pkt_encode((const pkt_t *) window[index_tmp],(char *) tabl,(size_t *) &mylong);
			gettimeofday((timer_buffer[last_timer_setup])->timer_t,0);
			write(fd_socket,(void *)tabl,mylong);
			free(tabl);
			}	
		}

		pkt_t *pkt = pkt_new();

		//Réinitialisation du timer
		timer->tv_sec=TIME;
		FD_ZERO(lecture);
		FD_SET(fd_socket, lecture);

		//vérification des données en lecture
		int timer_check = select(fd_socket+1,lecture,NULL,NULL,timer);
		if(timer_check>0){
			if(FD_ISSET(fd_socket, lecture)){
				//recuperation de la longueur lue
				length = read(fd_socket, (void *) packet_buffer, PACKET_MAX_SIZE);

				//decodage du packet
				if(length>0){
					pkt_status_code retour = pkt_decode((const char *) packet_buffer, (const size_t) length, pkt);
					if(retour==PKT_OK){
						//Si type ACK
						if((*pkt).mytype==PTYPE_ACK){
							rec_win=(*pkt).mywindow;
							if(index_offset==WINDOW_NBR-1){
								index_offset=0;
							}
							ack_to_seq_buffer((*pkt).myseqnum -1, 0,WINDOW_NBR);
								
							MIN_w = (*pkt).myseqnum-1;
							MAX_w = MIN_w + WINDOW_NBR;
							if(MAX_w>MAXSEQNUM){
								MAX_w=MAX_w-MAXSEQNUM;
							}
							if(MIN_w>MAXSEQNUM){
								MIN_w=MIN_w-MAXSEQNUM;
							}
		
						}
						
						//si type NACK
						if((*pkt).mytype==PTYPE_NACK){
							int index_temp=0;
							index_temp= look_for_seq_buffer((*pkt).myseqnum, 0, WINDOW_NBR);
							if(window[index_temp]!=NULL){//Corruption du num de séquence dans l'acquis 
								char *tab = malloc(PACKET_MAX_SIZE*sizeof(char));
								size_t mylength=(size_t) PACKET_MAX_SIZE;
								pkt_encode((const pkt_t *) window[index_temp],(char *) tab,(size_t *) &mylength);
								gettimeofday((timer_buffer[last_timer_setup])->timer_t, 0); //reinitialise le timer
								write(fd_socket,(void *)tab,mylength);
								free(tab);
							}					
						}
					}
					//Si condition de fin acquises, sortie du while
					if(!end_start && send_count==0){
						boolean=FALSE;
					}
					free(pkt->mydata);
					free(pkt);
				}
					
					
			}

		}

		//envoi possible de packet
		if(checking != WINDOW_NBR && end_start && dernierenvoi<MAX_w){
				//création des variables
				pkt_t *newpkt = pkt_new();
				Timer *time = malloc(sizeof(Timer));
				char *buff = malloc(PAYLOAD_MAX_SIZE*sizeof(char));

				//lecture du fichier				 
				int length = read(fd_filer, (void *) buff, PAYLOAD_MAX_SIZE);
				
	
				if(rec_win!=0){
					newpkt->mydata = malloc(length*sizeof(char));
					sum_readed = sum_readed+length;
					dernierenvoi++;
					//assignation des paramètre correspondant à la structure & insertion de la structure dans le buffer
 
					//assignation des paramètres
					memcpy((void *)newpkt->mydata,(void *)buff,length);
					(*newpkt).mytype=PTYPE_DATA;
					(*newpkt).mywindow=0;
					(*newpkt).myseqnum=dernierenvoi;
					(*newpkt).mylength=length;		
			
					//ajout buffer
					window[index_offset]=newpkt;
					timer_buffer[index_offset] = time;
					time->timer_t = malloc(sizeof(struct timeval));
					gettimeofday(timer_buffer[index_offset]->timer_t, 0);
					last_timer_setup = index_offset;
				
					
					char tab[PACKET_MAX_SIZE];
					size_t mylength=(size_t) PACKET_MAX_SIZE;
					pkt_encode((const pkt_t *) window[index_offset],(char *) tab, (size_t *) &mylength);

					
					write(fd_socket,(void *)tab,mylength);
					timer->tv_sec=TIME_s; //reinitialise le timer
					timer->tv_usec=TIME_m;
					checking++;
					send_count++;
					if(!(index_offset +1>=WINDOW_NBR)){
						index_offset++;
					} else {
						index_offset=0;
					}

					//si tout fichier lu envoi d'un packet de taille nulle
					if(sum_readed==file_length){
						pkt_t *newpkt = pkt_new();
						Timer *time = malloc(sizeof(Timer));
						char *bu = malloc(sizeof(char));
						dernierenvoi++;
						newpkt->mydata = bu;
						(*newpkt).mytype=PTYPE_DATA;
						(*newpkt).mywindow=0;
						(*newpkt).myseqnum=dernierenvoi;
						(*newpkt).mylength=0;

						window[index_offset]=newpkt;
						timer_buffer[index_offset] = time;
						time->timer_t = malloc(sizeof(struct timeval));
						gettimeofday((timer_buffer[index_offset])->timer_t, NULL);
						char tab[PACKET_MAX_SIZE];
						size_t mylength=(size_t) PACKET_MAX_SIZE;
						(int) pkt_encode((const pkt_t *) window[index_offset],(char *) tab, (size_t *) &mylength);

					
						write(fd_socket,(void *)tab,mylength);
						timer->tv_sec=TIME_s; //reinitialise le timer
						timer->tv_usec=TIME_m;
						checking++;
						send_count++;
						if(!(index_offset +1>=WINDOW_NBR)){
							index_offset++;
						} else {
							index_offset=0;
						}
						end_start=FALSE;
					}
					free(buff);
					
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
			check_file = 1;
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
		const char *err = "Error: Invalid port number parameter! Please enter a valid port number.\n";
		write(STDERR_FILENO, err, strlen(err));
	}
	
	//Récupération de l'adresse réelle IPv6
	struct sockaddr_in6 *serv_addr;
	
	struct sockaddr_in6 *local_addr;

	local_addr = malloc(sizeof(*local_addr));
	serv_addr = malloc(sizeof(*serv_addr));
	const char *ret = real_address(hostname, serv_addr);
	char *yolo = malloc(3*sizeof(char));
	strcpy(yolo,"::1");
	real_address(yolo, local_addr);
	char *def = malloc(3*sizeof(char));
	strcpy(def,"::1");
	
	if(ret!=NULL){
		write(STDERR_FILENO, ret, strlen(ret));
		exit(1);
	}

	const char *ph1 = "Connexion...";
	write(STDOUT_FILENO, ph1, strlen(ph1));
	//Création du socket
	int socket_client = create_socket(local_addr,10000,serv_addr, port);
	if(socket_client==-1){
		exit(-1);
	}

	const char *ph2 = "Done!\nSending file...";
	write(STDOUT_FILENO, ph2, strlen(ph2));

	//Redirection de stdin
	int fd_f;
	int back;
	if(check_file==1){
		//ouverture du fichier;
		fd_f = open(filename,O_RDWR, NULL);

		//prise de la taille;
		file_length =(int) lseek(fd_f,0L,SEEK_END);
		lseek(fd_f,0L,SEEK_SET);

	if(fd_f==-1){
		char *log = "Error: unable to read in the file\n";
		write(STDERR_FILENO,log, strlen(log));
		exit(1);
	}
	back = dup(STDIN_FILENO);
	fd_filer = dup2(fd_f, STDIN_FILENO);
	}
	
	//Envoi
	send_data(socket_client);

	//Backup
	if(check_file==1){
		dup2(back, STDIN_FILENO);
		close(fd_f);
	}
	close(socket_client);

	const char *ph3="Done!\n";
	write(STDOUT_FILENO, ph3, strlen(ph3));

	//free
	free_r();
	free(local_addr);
	free(serv_addr);
	free(yolo);
	free(def);
	
	return 0;
}


