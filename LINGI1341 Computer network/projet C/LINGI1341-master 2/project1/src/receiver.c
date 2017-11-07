//
//  main.c
//  LINGI1341-Project1
//
//  Created by Jsday on 14/10/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt
#include <getopt.h>
#include <zlib.h> //crc32
#include <string.h>
#include "packet_interface.h"
#include "real_address.h"
#include "read_write_loop.h"
#include "create_socket.h"
#include "wait_for_client.h"
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include "utils.h"
#include "queue.h"
#include "timer.h"
#include "receiver.h"

///////////////////////
#define TIME 1
#define TIME_s 1
#define TIME_m 150000
#define WINDOW_NBR 5
#define MAXPACKETSIZE 1024
#define PAYLOADSIZE 512
#define MAXSEQNUM 255
#define TRUE 1
#define FALSE 0
#define PTYPE_END		3
#define PTYPE_CLOSE		5
#define SR_TIMEOUT 		1
#define SEQNUMSIZE		1												// sequence number field length (in bytes)
#define MAXWINDOWSIZE 	31
#define MAXSEQNUM 		255
#define CONNECTIONTO	30												// timeout of the connection

void parse_parameters(int argc, char **argv);
int file_write(FILE* f);

int sfd;
struct sockaddr_in6 addr;
char opt;
char *file = NULL;
char *hostname = NULL;
int port = -1;
#define BUF_SIZE 1024

int last_sent;

/* worker threads */
pthread_t receiver;

/* sync goodies */
pthread_mutex_t buffer_lock, rs_lock, var_lock;

/* structures */
struct queue *buffer;

/* selective repeat variables */
int windowsize = MAXWINDOWSIZE;
int last_ack = 0;
int exp_seq_num = 0;													// expected sequence number

/* general timers */
struct timer_s *conn_t;													// connection timer

/* state variables */
int peer_to = FALSE;													// true if peer timed out
int end_ack = -1;														// PTYPE_END packet sequence number
int done = FALSE;														// when PTYPE_END has been received, this is true

//Il faut une variable globale qui contient le dernier numéro de séquence de reçu. Si le nouveau est l'ancien+1 alors on le change sinon, on renvoie un ack
//avec comme numéro de séquence (ancien+1) pour signaler la perte d'un packet.
int main(int argc, char ** argv){
    
    FILE* f = NULL;
    
    //Initialization
    parse_parameters(argc, argv);
    
    /* Resolve the hostname */
    const char *err = real_address(hostname, &addr);
    if (err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", hostname, err);
        return EXIT_FAILURE;
    }
    /* Get a socket */
    sfd = create_socket(&addr, port, NULL, -1);
    if(sfd < 0)
    {
        fprintf(stderr, "Failed to create the socket!\n");
        return EXIT_FAILURE;;
    }
    

    /*fprintf(stderr,"Wainting for a client..\n");
    int wait = wait_for_client(sfd);
    if(wait < 0)
    {
        fprintf(stderr,
                "Could not connect the socket after the first message.\n");
        close(sfd);
        return EXIT_FAILURE;;
    }*/
    
    
    /* No file specified */
    if (file == NULL) {
        /* Process I/O */
        read_write_loop(sfd);
    }
    else{
        /* Open for appending (writing at end of file) */
        f = fopen(file, "w");
        
        if(f < 0){
            fclose(f);
            close(sfd);
            return EXIT_FAILURE;
        }
        
        fclose(f);
    }
    
    
    struct timeval sock_timeout;
    sock_timeout.tv_sec = 0;
    sock_timeout.tv_usec = 100000;
    int res = setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));
    if (res == -1) {
        exit_error("could not set socket receive timeout");
    }
    
    /* init the receiver buffer */
    buffer = queue_create();
    if (buffer == NULL) {
        exit_error("could not create receiver buffer");
    }
    
    /* init timers */
    conn_t = timer_init();
    
    /* communication init */
    pkt_t *pckt = NULL;
    int seq_num, type;
    char raw_pckt[MAXPACKETSIZE];
    
    socklen_t address_len;
    address_len = sizeof(struct sockaddr_storage);
    /*res = bind(sock_desc, addr->ai_addr, addr->ai_addrlen);
    
    if (res == -1) {
        perror("error while binding in main");
        exit(EXIT_FAILURE);
    }*/
    
    /* LOOP START */
    while (1) {
        /* blocking call */
        //res = recvfrom(sock_desc, (void *) raw_pckt, MAXPACKETSIZE, 0, addr->ai_addr, &addr->ai_addrlen);
        
        ssize_t nread = recvfrom(sfd, raw_pckt, MAXPACKETSIZE, 0, (struct sockaddr *) &addr, &address_len);
        /*if(nread == EOF){
            printf("sfd: %zd\n", sfd);
            printf("read: %zd\n", nread);
            exit(EXIT_SUCCESS);
            break;
        }*/
        
        if (nread > -1) {
            printf("raw_pckt: %s\n", raw_pckt);
            pthread_mutex_lock(&var_lock);
            timer_refresh(conn_t); // refresh timer
            pthread_mutex_unlock(&var_lock);

            pckt = pkt_new();
            if (pckt == NULL) {
                break;
            }
            
            pkt_status_code code = pkt_decode(raw_pckt, sizeof(raw_pckt), pckt);
            printf("type: %d\n", pckt->type);
            printf("raw_pckt->payload: %s\n", pckt->payload);
            printf("size: %d\n", pckt->length);
            
            /* IS OUR PACKET VALID? */
            if (code == PKT_OK) {
                seq_num = pkt_get_seqnum(pckt);
                type = pkt_get_type(pckt);
                
                /* IS THE PACKET ALREADY IN THE BUFFER? */
                if (!is_in_buffer(seq_num)) {
                    /* INSIDE WINDOW? */
                    if (is_in_window(seq_num)) {
                        /* IN-ORDER? */
                        if (exp_seq_num == seq_num) {
                            printf("writing ... \n");
                            last_ack = write_pckts(pckt, f);
                            exp_seq_num = (last_ack+1) % (MAXSEQNUM+1);
                        }
                        else {
                            buffer_add(pckt);
                        }
                    
                        if (type == PTYPE_END) {
                            done = TRUE;
                            send_ack(PTYPE_END);
                        }
                        /* ARE WE DONE? */
                        else if ((type == PTYPE_CLOSE) && done) {
                            break;
                        }
                        else {
                            send_ack(PTYPE_ACK);
                        }
                    }
                    else {
                        /* if we're here and we're done, it means the
                         * ACK of the END packet was lost */
                        if (done) {
                            send_ack(PTYPE_END);
                        }
                    }
                }
                /* ACK for this packet must have been lost, resend last one */
                else {
                    send_ack(PTYPE_ACK);
                }
            }
            else {
                /* DID PEER TIMEOUT? */
                timer_stop(conn_t);
                if (timer_is_started(conn_t)) {
                    if (timer_get_secs(conn_t) >= CONNECTIONTO) {
                        empty_buffer();
                        if (!done) {
                            fprintf(stderr, "Peer timed out.\n");
                        }
                    
                        break;
                    }
                }
            }
        }
    /* LOOP END */
    }
    
    /* housekeeping */
    fclose(f);
    queue_destroy(buffer);
    freeaddrinfo((struct addrinfo *)&addr);
    timer_free(conn_t);
    
    close(sfd);
    
    return EXIT_SUCCESS;
    
}

int file_write(FILE* f){
    
    
    char buff[BUF_SIZE];
    ssize_t r = read(sfd, buff, BUF_SIZE);
    if(r == EOF){
        return 0;
    }
    
    int w = (int) fwrite(buff , 1 , r, f);
    
    if(w == -1){
        fprintf(stderr, "error with write (receiver)");
        return -1;
    }
    
    return 0;
}


void parse_parameters(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "`%s' arguments missing\n", argv[0]);
        fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
        exit(1);
    }
    else {
        /* specify which parameters we expect */
        static struct option options[] =
        {
            {"filename", required_argument, 0, 'f'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0}
        };
        
        while ((opt = getopt_long(argc, argv, "f:h:", options, NULL)) != -1) {
            switch (opt) {
                case 'f':
                    file = optarg;
                    break;
                case 'h':
                    printf("\n");
                    printf("Usage: %s hostname port\n", argv[0]);
                    printf("Usage: %s [-f X] hostname port\n", argv[0]);
                    printf("Usage: %s [--filename X] hostname port\n", argv[0]);
                    printf("\n");
                    exit(1);
                default:
                    fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                    exit(1);
            }
        }
        
        hostname = argv[argc-2];
        port = atoi(argv[argc-1]);
    }
    
}

void print_buffer() {
    int i;
    struct queue_element *qe;
    pkt_t *pckt;
    printf("======== BUFFER START ========\n");
    qe = buffer->front;
    for (i = 0; i < buffer->length; i++) {
        pckt = (pkt_t *) qe->element;
        printf("%d\n", pkt_get_seqnum(pckt));
        printf(" | \n");
        qe = qe->previous;
    }
    printf("======== BUFFER END ========\n");
}
pkt_t *buffer_next() {
    struct queue_element *qe;
    qe = queue_next(buffer);
    pkt_t *pckt = (pkt_t *) qe->element;
    free(qe);
    return pckt;
}

void empty_buffer() {
    int i = 0;
    int length = buffer->length;
    for (i = 0; i < length; i++) {
        pkt_del(buffer_next());
    }
}

struct queue_element *get_elem_from_buffer(int seq_num) {
    if (buffer->length == 0) {
        return NULL;
    }
    
    int i;
    struct queue_element *qe;
    pkt_t *pckt;
    
    qe = buffer->front;
    pckt = (pkt_t *) qe->element;
    
    for (i = 0; i < buffer->length; i++) {
        if (pkt_get_seqnum(pckt) == seq_num) {
            return qe;
        }
        
        qe = qe->previous;
        if (qe != NULL) {
            pckt = (pkt_t *) qe->element;
        }
    }
    
    return NULL;
}

pkt_t *get_pckt_from_buffer(int seq_num) {
    struct queue_element *qe;
    pkt_t *pckt;
    
    qe = get_elem_from_buffer(seq_num);
    if (qe != NULL) {
        pckt = (pkt_t *) qe->element;
        if (qe->previous != NULL) {
            qe->previous->next = qe->next;
        }
        else {
            buffer->rear = qe->next;
        }
        if (qe->next != NULL) {
            qe->next->previous = qe->previous;
        }
        else {
            buffer->front = qe->previous;
        }
        
        free(qe);
        buffer->length--;
        return pckt;
    }
    
    return NULL;
}

void buffer_add(pkt_t *pckt) {
    struct queue_element *qe = (struct queue_element *) malloc(sizeof(struct queue_element));
    if (qe == NULL) {
        exit_error("could not create queue element");
    }
    qe->element = (void *) pckt;
    
    queue_add(buffer, qe);
}

int write_pckts(pkt_t *pckt, FILE *fp) {
    int i = exp_seq_num;
    
    while (pckt != NULL) {
        fwrite(pckt->payload, pkt_get_length(pckt), 1, fp);
        pkt_del(pckt);
        i = (i+1) % (MAXSEQNUM+1);
        pckt = get_pckt_from_buffer(i);
    }
    
    i = (i-1) % (MAXSEQNUM+1);
    return i;
}

void write_buff_to_pckt(pkt_t *pckt) {
    int i, buff_size = buffer->length;
    struct queue_element *qe;
    pkt_t *tmp;
    qe = buffer->front;
    for (i = 0; i < buff_size; i++) {
        tmp = (pkt_t *) qe->element;
        proto_write_to_pl(pckt, i*SEQNUMSIZE, pkt_get_seqnum(tmp), SEQNUMSIZE);
        qe = qe->previous;
    }
    pkt_set_length(pckt, buff_size);
}

void send_ack(int type) {
    int buff_size = buffer->length;
    windowsize = MAXWINDOWSIZE - buff_size;
    pkt_t *pckt = proto_create_pckt(NULL, type, windowsize, exp_seq_num, 0);
    write_buff_to_pckt(pckt);											// make it a selective ACK
    //proto_update_crc(pckt);
    proto_send_pckt(pckt);
    pkt_del(pckt);
}

int is_in_window(int seq_num) {
    int lower_bound = exp_seq_num;
    int upper_bound = (last_ack+windowsize) % (MAXSEQNUM+1);
    return is_in_interval(lower_bound, upper_bound, seq_num);
}

int is_in_buffer(int seq_num) {
    struct queue_element *elem = get_elem_from_buffer(seq_num);
    return elem == NULL ? 0 : 1;
}

void proto_write_to_pl(pkt_t *pckt, int offset, uint32_t bits, int length) {
    bits = htonl(bits);
    memcpy(pckt->payload + offset, &bits, length);
}

pkt_t *create_pckt(char *data, int type, int length) {
    return proto_create_pckt(data, type, 0, last_sent, length);
}

pkt_t *proto_create_pckt(char *data, int type, int window, int seqnum, int length) {
    
    pkt_t *pckt = pkt_new();
    printf("init a packet .. \n");
    if (pckt == NULL) {
        fprintf(stderr,"init a pkt\n");
        exit(EXIT_FAILURE);
    }
    printf("init a packet .. end \n");
    pkt_set_type(pckt, type);
    pkt_set_window(pckt, window);
    pkt_set_seqnum(pckt, seqnum);
    pkt_set_length(pckt, length);
    printf("init a packet .. end 2 \n");
    if (data != NULL) {
        printf("data :%s\n", data);
        printf("length :%d\n", length);
        pckt->payload = malloc(length);
        if (pckt->payload == NULL) {
            fprintf(stderr,"cannot allocate ..\n");
            exit(EXIT_FAILURE);
        }
        memcpy(pckt->payload, data, length);
        printf("payload :%s\n", pckt->payload);
    }
    //proto_update_crc(pckt);
    
    pckt->timer = timer_init();
    
    return pckt;
}
void proto_send_pckt(pkt_t *pckt) {
    
    char raw_pckt[MAXPACKETSIZE];
    
    size_t *len = malloc(sizeof(size_t));
    *len = sizeof(raw_pckt);
    printf("length :%zu\n", *len);
    
    pkt_status_code code = pkt_encode(pckt, raw_pckt, len);
    
    if (code == PKT_OK) {
        //struct sockaddr_storage address;
        //socklen_t address_len;
        //address_len = sizeof(struct sockaddr_storage);
        
        int w = (int) write(sfd,raw_pckt,*len);
        if(w == -1){
            fprintf(stderr, "error with write (read_write_loop)");
        }
        /*int res = sendto(sock_desc, raw_pckt, *len, 0, (struct sockaddr *)send_ai, sizeof(send_ai));
         if (res == -1) {
         perror("error while sending packet (send_packet)");
         exit(EXIT_FAILURE);
         }
         
         if (pckt->timer != NULL) {
         timer_start(pckt->timer);
         }*/
    }
    exit(EXIT_SUCCESS);
    /*uint32_t header = htonl(pckt->header); 								// careful on the endianess
     uint32_t crc = htonl(pckt->crc); 									// careful on the endianess
     bzero(raw_pckt, MAXPACKETSIZE);
     
     merge packet fields into the raw packet
     memcpy(raw_pckt, &header, HEADERSIZE);
     memcpy(raw_pckt+HEADERSIZE, pckt->payload, PAYLOADSIZE);
     memcpy(raw_pckt+HEADERSIZE+PAYLOADSIZE, &crc, CRCSIZE);*/
    
}


