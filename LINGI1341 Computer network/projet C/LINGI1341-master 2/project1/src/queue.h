
struct queue_element {
	void *element;
	struct queue_element *previous;
	struct queue_element *next;
};

struct queue {
	struct queue_element *front;
	struct queue_element *rear;
	int length;
};

/* returns NULL if error, pointer otherwise */
struct queue *queue_create();

void queue_add(struct queue *q, struct queue_element *qe);

/* removes next element from queue and returns it, if queue is empty
 * returns NULL */
struct queue_element *queue_next(struct queue *q);

int queue_is_empty(struct queue *q);

void queue_destroy(struct queue *q);
