
#include "packet_interface.h"

void parse_params(int argc, char **argv);

void send_pckt(int sock_desc, pkt_t *pckt, int resend);

/* thread-safe */
pkt_t *get_pckt_from_buffer(int seq_num);

void rs_remove(int seq_num);

void print_buffer();

/* thread-safe */
void check_for_timeouts();

/* thread-safe */
void rs_add(pkt_t *pckt);

/* thread-safe */
int rs_is_empty();

struct queue_element *get_elem_from_rs(int seq_num);

/* thread-safe */
pkt_t * rs_next();

/* thread-safe */
void buffer_add(pkt_t *pckt);

/* thread-safe */
pkt_t *buffer_next();

/* thread-safe */
void empty_buffer();

void selective_ack(pkt_t *pckt);

void *receiver_routine();

int is_in_window(int seq_num);

int how_many_acks(int new_ack, int last_ack);

static inline void print_progress(long x, long total, int freq, int width, char *msg);

/* thread-safe */
int are_we_done();

/* thread-safe */
void set_reading_done(int value);

/* thread-safe */
void set_receiver_done(int value);

/* thread-safe */
int did_peer_to();

/* thread-safe */
void set_peer_to(int value);

/* thread-safe */
int buffer_is_empty();

/* thread-safe */
int get_buffer_length();

/* thread-safe */
int get_windowsize();

/* thread-safe */
void set_windowsize(int value);

void increment_seq_num();

/* thread-safe */
void increment_total_sent();

/* thread-safe */
long get_total_sent();

/* thread-safe */
int is_in_rs(int seq_num);

pkt_t *create_pckt(char *data, int type, int length);
pkt_t *proto_pckt_cpy(pkt_t *pckt);
void proto_send_pckt(int sock_desc, pkt_t *pckt);
uint32_t proto_read_from_pl(pkt_t *pckt, int offset, int length);
pkt_t *proto_create_pckt(char *data, int type, int window, int seq_num, int length);
struct addrinfo *get_addr_info(char *host);
