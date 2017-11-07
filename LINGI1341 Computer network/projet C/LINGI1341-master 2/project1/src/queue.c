/*
 * Generic queue
 * 
 * Copyright 2014 Joey Moubarak <hey@jwy.be>
 * 
 */

#include <stdlib.h>
#include "queue.h"

struct queue *queue_create() {
	struct queue *q = (struct queue *) malloc(sizeof(struct queue));
	if (q == NULL) {
		return NULL;
	}
	else {
		q->length = 0;
		q->front = NULL;
		q->rear = NULL;
	}
	return q;
}

void queue_add(struct queue *q, struct queue_element *qe) {
	qe->previous = NULL;
	qe->next = q->rear;
	if (!queue_is_empty(q)) {
		q->rear->previous = qe;
	}
	else {
		q->front = qe;
	}
	q->rear = qe;
	q->length++;
}

struct queue_element *queue_next(struct queue *q) {
	if (queue_is_empty(q)) {
		return NULL;
	}
	
	struct queue_element *qe = q->front;
	struct queue_element *previous = qe->previous;
	if (previous != NULL) {
		previous->next = qe->next;
	}
	else {
		q->rear = NULL;
	}
	q->front = qe->previous;
	q->length--;
	return qe;
}

int queue_is_empty(struct queue *q) {
	return (q->length == 0) ? 1 : 0;
}

void queue_destroy(struct queue *q) {
	free(q);
}
