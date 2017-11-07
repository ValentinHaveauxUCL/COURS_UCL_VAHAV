
void parse_params(int argc, char **argv);

void print_buffer();

pkt_t *buffer_next();

void empty_buffer();

int is_in_buffer();

/* gets a specific packet from the buffer and returns it */
struct queue_element *get_elem_from_buffer(int seq_num);

/* removes a specific packet from the buffer and returns it */
pkt_t *get_pckt_from_buffer(int seq_num);

void buffer_add(pkt_t *pckt);

/* write in-order packets that are in the buffer to the file
 * returns the last in-order packet sequence number */
int write_pckts(pkt_t *pckt, FILE *fp);

void write_buff_to_pckt(pkt_t *pckt);

void send_ack(int type);

int is_in_window(int seq_num);
void proto_write_to_pl(pkt_t *pckt, int offset, uint32_t bits, int length);
pkt_t *create_pckt(char *data, int type, int length);
pkt_t *proto_create_pckt(char *data, int type, int window, int seqnum, int length);
pkt_t *proto_create_pckt(char *data, int type, int window, int seqnum, int length);
void proto_send_pckt(pkt_t *pckt);
