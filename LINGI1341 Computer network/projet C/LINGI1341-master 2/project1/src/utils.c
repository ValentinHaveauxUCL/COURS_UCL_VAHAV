
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "utils.h"

void exit_error(char *msg) {
	fprintf(stderr, "Error: %s. Exiting.\n", msg);
	exit(EXIT_FAILURE);
}

int is_in_interval(int a, int b, int val) {
	if (a < b) {
		if (a <= val && val <= b) {
			return 1;
		}
	}
	if (a > b) {
		if (a <= val || val <= b) {
			return 1;
		}
	}
	if (a == b) {
		if (val == a) {
			return 1;
		}
	}
	return 0;
}

int how_many_in_interval(int a, int b, int size) {
	if (a > b) {
		int rem = size-a;												// remainder before end of interval of length 'size'
		return rem+b;
	}
	else {
		return b-a;
	}
}

float eval_speed(long x, struct timer_s *timer) {
	timer_stop(timer);
	int secs = timer_get_secs(timer);
	if (secs != 0) {
		return ((x*512)/1024)/secs;
	}
	else {
		return 0;
	}
}

