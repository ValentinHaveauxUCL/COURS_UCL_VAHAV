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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "utils.h"
#include "queue.h"
#include "timer.h"
#include "sender.h"

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


/* Global variable */
char opt;
char *file = NULL;
char *hostname = NULL;
char *port = NULL;
int sfd;
//pkt_t * window[32];
char *buf;

//struct sockaddr_in6 *send_ai;
struct addrinfo *rcv_ai;

int sber = 0;
int splr = 0;
int delay = 0;
char *hn = NULL;
int verbose = FALSE;

/* structures */
struct queue *buffer;
struct queue *resend;

/* worker threads */
pthread_t receiver;

/* sync goodies */
pthread_mutex_t buffer_lock, rs_lock, var_lock;

/* selective repeat variables */
int windowsize;															// this is our max window capacity at any time
int last_sent;
int last_ack;

/* general timers */
struct timer_s *conn_t;													// connection timer
//struct timeval persist_t;												// persistence timer
struct timer_s *speed_t;

/* state variables */
int reading_done = FALSE;												// when sender has finished reading file, this is true
int receiver_done = FALSE;												// when receiver thread has received PTYPE_END ACK packet, this is true
int peer_to = FALSE;

/* statistics */
struct stat file_info;
long total_acked = 0;
long total_sent = 0;
long total_pckts = 0;
float speed = 0;

#define BUF_SIZE 500
#define MAX_SIZE 524

void parse_parameters(int argc, char **argv);
int file_read(int sfd, int fd);


int main(int argc, char **argv){
    
    FILE *fp;
    
    //Initialization
    parse_parameters(argc, argv);
    
    /* Resolve the hostname */
    struct sockaddr_in6 addr;
    const char *err = real_address(hostname, &addr);
    if (err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", hostname, err);
        return EXIT_FAILURE;
    }

    /* Get a socket */
    
    sfd = create_socket(NULL, -1, &addr, atoi(port)); /* Connected */
    if(sfd < 0)
    {
        fprintf(stderr, "Failed to create the socket!\n");
        return EXIT_FAILURE;
    }
    
    rcv_ai = get_addr_info( "::");
    
    /* set timeout */
    struct timeval sock_timeout;
    sock_timeout.tv_sec = 0;
    sock_timeout.tv_usec = 100000;
    int res = setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));
    if (res == -1) {
        exit_error("could not set socket receive timeout");
    }
    
    /* init the sender buffer */
    buffer = queue_create();
    if (buffer == NULL) {
        exit_error("could not create sender buffer");
    }
    
    /* init the resend queue */
    resend = queue_create();
    if (resend == NULL) {
        exit_error("could not create resend queue");
    }
    
    /* init the timers */
    pthread_mutex_lock(&var_lock);
    conn_t = timer_init();
    speed_t = timer_init();
    timer_start(conn_t);
    timer_start(speed_t);
    pthread_mutex_unlock(&var_lock);
    
    /* init sync goodies */
    pthread_mutex_init(&buffer_lock, NULL);
    pthread_mutex_init(&rs_lock, NULL);
    pthread_mutex_init(&var_lock, NULL);
    
    /* create receiver thread */
    if (pthread_create(&receiver, NULL, receiver_routine, NULL) != 0) {
        exit_error("could not create receiver thread");
    }

    
    /* First message
    char *hello = "hello";
    write(sfd,hello,sizeof(hello));*/
    
    /* No file specified */
    if (file == NULL) {
        printf("Write a message to send:\n");
        /* Process I/O */
        read_write_loop(sfd);
    }
    else{
        /* init input */
        fp = fopen(file, "r");
        
        if(fp==NULL)
		{
	            fprintf(stderr,"could not read file\n");
	            fclose(fp);
	            close(sfd);
	            return EXIT_FAILURE;
		}

    }
    
    int rd;	// read bytes counter
    char chunk[PAYLOADSIZE]; // read buffer
    
    /* init connection */
    set_windowsize(1);
    last_sent = 0;
    last_ack = 0;
    pkt_t *pckt;
    
    while (!are_we_done()) {
        /* ANY TIMEOUT? */
        if (!buffer_is_empty()) {
            check_for_timeouts();
        }
        
        /* check the persistence timer_*/
        
        
        /* check if peer timed out */
        if (did_peer_to()) {
            break;
        }
        
        /* SLIDING WINDOW/BUFFER FULL? */
        if (get_windowsize() > get_buffer_length()) {
            /* NEXT PACKET IS IN WINDOW? */
            if (is_in_window(last_sent)) {
                /* DID WE FINISH READING THE FILE? */
                if (!reading_done) {
                    /* if we reached end of file, send PTYPE_END packet */
                    if (feof(fp)) {
                        set_reading_done(TRUE);
                        pckt = create_pckt(NULL, PTYPE_END, 0);
                        pckt = pkt_new();
                    }
                    else {
                        rd = fread(chunk, 1, PAYLOADSIZE, fp);
                        pckt = create_pckt(chunk, PTYPE_DATA, rd);
                    }
                    
                    send_pckt(sfd, pckt, FALSE);
                    increment_total_sent();
                }
            }
        }
        
        else if (!rs_is_empty()) {
            pckt = rs_next();
            if (get_pckt_from_buffer(pkt_get_seqnum(pckt)) != NULL) {
                printf("Resending pckt %d\n", pkt_get_seqnum(pckt));
                send_pckt(sfd, pckt, TRUE);
                printf("Resent pckt %d\n", pkt_get_seqnum(pckt));
            }
            
            increment_total_sent();
            if (pkt_get_type(pckt) == PTYPE_ACK) {
                pkt_del(pckt);
                break;
            }
        }
    }


    fclose(fp);
    close(sfd);
    return EXIT_SUCCESS;
    
}

/*int file_read(int sfd, int fd){
    char buffer[BUF_SIZE];
    ssize_t r = read(fd, buffer, sizeof(buffer));
    
    int w = (int) write(sfd,buffer,r);
    if(w == -1){
        fprintf(stderr, "error with write (sender)");
        return -1;
    }

    return 0;
}*/


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
        port = argv[argc-1];
    }
    
}

void send_pckt(int sock_desc, pkt_t *pckt, int resendID) {
    
    pkt_t *pckt_copy = proto_pckt_cpy(pckt);				// we don't want to mess with our original packet
    
    pckt_copy->payload = malloc(pckt->length);
    if (pckt_copy->payload == NULL) {
        fprintf(stderr,"cannot allocate ..\n");
        exit(EXIT_FAILURE);
    }
    /* are we resending a packet? */
    if (!resendID) {
        buffer_add(pckt);
        increment_seq_num();
    }

    /* simulate packet invalidity */
    if (random()%1000 < sber) {
        pckt_copy->payload[0] ^= 0xff;
    }
    
    /* simulate packet loss */
    if (random()%100 > splr) {
        proto_send_pckt(sock_desc, pckt_copy);
    }
    
    timer_start(pckt->timer);											// we sent a copy, so we have to manually start
    // the timer of the original packet
    pkt_del(pckt_copy);
    exit(EXIT_SUCCESS);
}

pkt_t *get_pckt_from_buffer(int seq_num) {
    pthread_mutex_lock(&buffer_lock);
    if (buffer->length == 0) {
        pthread_mutex_unlock(&buffer_lock);
        return NULL;
    }
    
    int i;
    struct queue_element *qe;
    pkt_t *pckt;
    
    qe = buffer->front;
    pckt = (pkt_t *) qe->element;
    
    for (i = 0; i < buffer->length; i++) {
        if (pkt_get_seqnum(pckt) == seq_num) {
            pthread_mutex_unlock(&buffer_lock);
            return pckt;
        }
        
        qe = qe->previous;
        if (qe != NULL) {
            pckt = (pkt_t *) qe->element;
        }
    }
    
    pthread_mutex_unlock(&buffer_lock);
    
    return NULL;
}

struct queue_element *get_elem_from_rs(int seq_num) {
    pthread_mutex_lock(&rs_lock);
    if (resend->length == 0) {
        pthread_mutex_unlock(&rs_lock);
        return NULL;
    }
    
    int i;
    struct queue_element *qe = NULL;
    pkt_t *pckt;
    
    qe = resend->front;
    pckt = (pkt_t *) qe->element;
    
    for (i = 0; i < resend->length; i++) {
        pckt = (pkt_t *) qe->element;
        if (pkt_get_seqnum(pckt) == seq_num) {
            break;
        }
        
        qe = qe->previous;
    }
    
    pthread_mutex_unlock(&rs_lock);
    
    return qe;
}

void rs_remove(int seq_num) {
    struct queue_element *qe;
    //pkt_t *pckt;
    
    qe = get_elem_from_rs(seq_num);
    
    pthread_mutex_lock(&rs_lock);
    if (qe != NULL) {
        //pckt = (pkt_t *) qe->element;
        if (qe->previous != NULL) {
            qe->previous->next = qe->next;
        }
        else {
            resend->rear = qe->next;
        }
        if (qe->next != NULL) {
            qe->next->previous = qe->previous;
        }
        else {
            resend->front = qe->previous;
        }
        
        free(qe);
        resend->length--;
    }
    pthread_mutex_unlock(&rs_lock);
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

void check_for_timeouts() {
    int i;
    struct queue_element *qe;
    pkt_t *pckt;
    
    pthread_mutex_lock(&buffer_lock);
    
    qe = buffer->front;
    
    for (i = 0; i < buffer->length; i++) {
        pckt = (pkt_t *) qe->element;
        if (timer_is_started(pckt->timer)) {
            timer_stop(pckt->timer);
            if (timer_get_secs(pckt->timer) >= SR_TIMEOUT) {
                timer_reset(pckt->timer);								// reset packet timer
                rs_add(pckt);
            }
            
        }
        
        qe = qe->previous;
    }
    
    pthread_mutex_unlock(&buffer_lock);
}

void rs_add(pkt_t *pckt) {
    if (is_in_rs(pkt_get_seqnum(pckt))) {
        return;
    }
    struct queue_element *qe = (struct queue_element *) malloc(sizeof(struct queue_element));
    if (qe == NULL) {
        exit_error("could not create queue element");
    }
    qe->element = (void *) pckt;
    
    pthread_mutex_lock(&rs_lock);
    queue_add(resend, qe);
    pthread_mutex_unlock(&rs_lock);
}

pkt_t *rs_next() {
    struct queue_element *qe;
    pthread_mutex_lock(&rs_lock);
    qe = queue_next(resend);
    pthread_mutex_unlock(&rs_lock);
    pkt_t *pckt = (pkt_t *) qe->element;
    free(qe);
    return pckt;
}

void buffer_add(pkt_t *pckt) {
    struct queue_element *qe = (struct queue_element *) malloc(sizeof(struct queue_element));
    if (qe == NULL) {
        exit_error("could not create queue element");
    }
    qe->element = (void *) pckt;
    
    pthread_mutex_lock(&buffer_lock);
    queue_add(buffer, qe);
    pthread_mutex_unlock(&buffer_lock);
}

pkt_t *buffer_next() {
    struct queue_element *qe;
    pthread_mutex_lock(&buffer_lock);
    qe = queue_next(buffer);
    pthread_mutex_unlock(&buffer_lock);
    pkt_t *pckt = (pkt_t *) qe->element;
    free(qe);
    return pckt;
}

void empty_buffer() {
    int i = 0;
    pthread_mutex_lock(&buffer_lock);
    int length = buffer->length;
    pthread_mutex_unlock(&buffer_lock);
    for (i = 0; i < length; i++) {
        pkt_del(buffer_next());
    }
}

void selective_ack(pkt_t *pckt) {
    pkt_t *tmp;
    int i, seq_num, length = pkt_get_length(pckt);
    /* reset timer of each packet that was selectively ACKed */
    for (i = 0; i < length; i++) {
        seq_num = (int) proto_read_from_pl(pckt, i*SEQNUMSIZE, SEQNUMSIZE);
        tmp = get_pckt_from_buffer(seq_num);
        if (tmp != NULL) {
            timer_reset(tmp->timer);
        }
    }
}

void *receiver_routine() {
    int dup = 0, cu_ack, new_ack, i, type, res;
    char raw_pckt[MAXPACKETSIZE];
    pkt_t *pckt = NULL;
    
    while (TRUE) {
        /* blocking call */
        res = recvfrom(sfd, (void *) raw_pckt, MAXPACKETSIZE, 0, rcv_ai->ai_addr, &rcv_ai->ai_addrlen);
        if (res > 0) {
            pthread_mutex_lock(&var_lock);
            timer_refresh(conn_t);										// refresh timer
            pthread_mutex_unlock(&var_lock);
            
            //pckt = proto_parse_pckt(raw_pckt);
            pkt_status_code code = pkt_decode(raw_pckt, sizeof(raw_pckt), pckt);
            
            /* IS OUR PACKET VALID? */
            if (code == PKT_OK) {
                new_ack = pkt_get_seqnum(pckt);
                type = pkt_get_type(pckt);
                set_windowsize(pkt_get_window(pckt));
                
                /* DUPLICATE ACK? */
                if (new_ack == last_ack) {
                    if (dup == 3) {
                        pkt_t *rs_pckt = get_pckt_from_buffer(new_ack);
                        rs_add(rs_pckt);
                    }
                    dup++;
                }
                else if (is_in_window(new_ack)) {
                    dup = 1;											// reset duplicate counter
                    cu_ack = how_many_acks(new_ack, last_ack);
                    pkt_t *acked;
                    
                    /* reset timers of selected packets so they won't
                     * be resent */
                    selective_ack(pckt);
                    
                    /* remove ack'ed packets from buffer and resend stack and free them */
                    for (i = 0; i < cu_ack; i++) {
                        acked = buffer_next();
                        rs_remove(pkt_get_seqnum(acked));
                        if (get_elem_from_rs(pkt_get_seqnum(acked)) == NULL) {
                            pkt_del(acked);
                        }
                        total_acked++;
                    }
                    
                    last_ack = new_ack;
                    
                    /* print progress bar */
                    if (file == NULL) {									// reading from stdin
                        print_progress(0, 0, 0, 50, "uploading...");
                    }
                    else {
                        print_progress(total_acked, total_pckts, total_pckts, 50, "uploading...");
                    }
                    
                    /* DID WE RECEIVE PTYPE_END PACKET? */
                    if (type == PTYPE_END) {
                        /* DID WE RECEIVE ALL THE ACKS? */
                        if (buffer_is_empty()) {
                            pkt_t *close = create_pckt(NULL, PTYPE_CLOSE, 0);
                            rs_add(close);
                            set_receiver_done(TRUE);
                            pkt_del(pckt);						// ACK packet not needed anymore
                            break;
                        }
                    }
                }
            }
            
            pkt_del(pckt);										// ACK packet not needed anymore
        }
        else {
            /* if peer timed out, stop our job */
            timer_stop(conn_t);
            if (timer_get_secs(conn_t) >= CONNECTIONTO) {
                empty_buffer();
                set_peer_to(TRUE);
                break;
            }
        }
    }
    
    return NULL;
}

int is_in_window(int seq_num) {
    int lower_bound = last_ack % (MAXSEQNUM+1);
    int upper_bound = (last_ack+get_windowsize()) % (MAXSEQNUM+1);
    return is_in_interval(lower_bound, upper_bound, seq_num);
}

int how_many_acks(int new_ack, int last_ackID) {
    return how_many_in_interval(last_ackID, new_ack, MAXSEQNUM+1);
}

/* Inspired from http://bit.ly/1323c41 */
static inline void print_progress(long x, long total, int freq, int width, char *msg) {
    /* refresh line every '1/freq' times */
    if (x % (total/freq) != 0) {
        return;
    }
    
    float ratio = x/(float)total;
    int c = ratio * width;
    
    /* print percentage */
    printf("%3d%% [", (int) (ratio*100) );
    
    /* print the progress bar */
    for (x=0; x<c; x++) {
        printf("=");
    }
    
    for (x=c; x<width; x++) {
        printf(" ");
    }
    
    speed = eval_speed(get_total_sent(), speed_t);
    
    if (x == total) {
        printf("] [%s]", msg);
        int i;
        for (i=0; i<20; i++) {
            printf(" ");
        }
    }
    else {
        printf("] [%s] [%.1f KB/s]", msg, speed);
        printf("    ");
    }
    
    printf("\r");
    fflush(stdout);
}

int are_we_done() {
    pthread_mutex_lock(&var_lock);
    int done = (reading_done && receiver_done) && rs_is_empty();
    pthread_mutex_unlock(&var_lock);
    return done;
}

void set_reading_done(int value) {
    pthread_mutex_lock(&var_lock);
    reading_done = value;
    pthread_mutex_unlock(&var_lock);
}

void set_receiver_done(int value) {
    pthread_mutex_lock(&var_lock);
    receiver_done = value;
    pthread_mutex_unlock(&var_lock);
}

int did_peer_to() {
    pthread_mutex_lock(&var_lock);
    int to = peer_to;
    pthread_mutex_unlock(&var_lock);
    return to;
}

void set_peer_to(int value) {
    pthread_mutex_lock(&var_lock);
    peer_to = value;
    pthread_mutex_unlock(&var_lock);
}

int buffer_is_empty() {
    pthread_mutex_lock(&buffer_lock);
    int is_empty = queue_is_empty(buffer);
    pthread_mutex_unlock(&buffer_lock);
    return is_empty;
}

int get_buffer_length() {
    pthread_mutex_lock(&buffer_lock);
    int length = buffer->length;
    pthread_mutex_unlock(&buffer_lock);
    return length;
}

int rs_is_empty() {
    pthread_mutex_lock(&rs_lock);
    int is_empty = queue_is_empty(resend);
    pthread_mutex_unlock(&rs_lock);
    return is_empty;
}

int get_windowsize() {
    pthread_mutex_lock(&var_lock);
    int value = windowsize;
    pthread_mutex_unlock(&var_lock);
    return value;
}

void set_windowsize(int value) {
    pthread_mutex_lock(&var_lock);
    windowsize = value;
    pthread_mutex_unlock(&var_lock);
}

void increment_seq_num() {
    last_sent = (last_sent+1) % (MAXSEQNUM+1);
}

void increment_total_sent() {
    pthread_mutex_lock(&var_lock);
    total_sent++;
    pthread_mutex_unlock(&var_lock);
}

long get_total_sent() {
    pthread_mutex_lock(&var_lock);
    long value = total_sent;
    pthread_mutex_unlock(&var_lock);
    return value;
}

int is_in_rs(int seq_num) {
    struct queue_element *qe = get_elem_from_rs(seq_num);
    return (qe == NULL) ? FALSE : TRUE;
}

pkt_t *create_pckt(char *data, int type, int length) {
    return proto_create_pckt(data, type, 0, last_sent, length);
}

pkt_t *proto_pckt_cpy(pkt_t *pckt) {

    pkt_t *copy = (pkt_t *) malloc(sizeof(pckt));
    if (copy == NULL) {
        exit_error("could not allocate memory for packet copy");
        exit(EXIT_FAILURE);
    }
    
    bzero(copy, sizeof(&pckt));
    
    size_t len = sizeof(pckt);
    memcpy(copy, pckt, len);
    
    return copy;
}

void proto_send_pckt(int sock_desc, pkt_t *pckt) {
    
    char raw_pckt[MAXPACKETSIZE];
    
    size_t *len = malloc(sizeof(size_t));
    *len = sizeof(raw_pckt);
    
    pkt_status_code code = pkt_encode(pckt, raw_pckt, len);
    
    if (code == PKT_OK) {
        //struct sockaddr_storage address;
        //socklen_t address_len;
        //address_len = sizeof(struct sockaddr_storage);
        
        fprintf(stdout,"Sending ... \n");
        int w = (int) write(sock_desc,raw_pckt,*len);
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

uint32_t proto_read_from_pl(pkt_t *pckt, int offset, int length) {
    uint32_t bits;
    memcpy(pckt->payload + offset, &bits, length);
    bits = ntohl(bits);
    return bits;
}

pkt_t *proto_create_pckt(char *data, int type, int window, int seqnum, int length) {
    printf("create a packet .. \n");
    pkt_t *pckt = pkt_new();
    if (pckt == NULL) {
        fprintf(stderr,"init a pkt\n");
        exit(EXIT_FAILURE);
    }

    pkt_set_type(pckt, type);
    pkt_set_window(pckt, window);
    pkt_set_seqnum(pckt, seqnum);
    pkt_set_length(pckt, length);

    if (data != NULL) {

        pckt->payload = malloc(length);
        if (pckt->payload == NULL) {
            fprintf(stderr,"cannot allocate ..\n");
            exit(EXIT_FAILURE);
        }
        memcpy(pckt->payload, data, length);
    }
    //proto_update_crc(pckt);
    
    pckt->timer = timer_init();
    
    return pckt;
}

/*void proto_update_crc(pkt_t *pckt) {
    uint32_t crc = compute_crc(pckt);
    pckt->crc = crc;
}

uint32_t compute_crc(pkt_t *pckt) {
    int tmp_size = HEADERSIZE+PAYLOADSIZE;
    unsigned char tmp[tmp_size];
    uint32_t header = htonl(pckt->header);
    memcpy(tmp, &header, HEADERSIZE);
    memcpy(tmp+HEADERSIZE, pckt->payload, PAYLOADSIZE);
    return crc32(0, tmp, tmp_size);
}*/

struct addrinfo *get_addr_info(char *host) {
    /* get address structure from info we have */
    struct addrinfo hints;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    
    struct addrinfo *addr;
    struct addrinfo *list;
    
    int res = getaddrinfo(host, port, &hints, &list);
    if (res != 0) {
        perror("error while creating address structure (create_udp_sock)");
        exit(EXIT_FAILURE);
    }
    
    /* we only take the first result of our list (probably the right one)
     * we also set the 'next' field to NULL because it is no uint32_ter in
     * a list (that way we can use freeaddrinfo later) */
    addr = list;
    addr->ai_next = NULL;
    
    /* cleanup starting with the second element of list so we don't lose
     * our address structure */
    freeaddrinfo(list->ai_next);
    
    return addr;
}


