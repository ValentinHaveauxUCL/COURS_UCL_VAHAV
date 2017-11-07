#include <stdlib.h>
#include <sys/time.h>
#include "utils.h"
#include "timer.h"

struct timer_s *timer_init() {
	struct timer_s *timer = (struct timer_s *) malloc(sizeof(struct timer_s));
	if (timer == NULL) {
		exit_error("could not init timer");
	}
	
	struct timeval *start = (struct timeval *) malloc(sizeof(struct timeval));
	struct timeval *end = (struct timeval *) malloc(sizeof(struct timeval));
	if (start == NULL || end == NULL) {
		exit_error("could not allocate memory for timers");
	}
	
	timer->start = start;
	timer->end = end;
	
	timer_reset(timer);
	
	return timer;
}

void timer_start(struct timer_s *timer) {
	gettimeofday(timer->start, NULL);
}

void timer_stop(struct timer_s *timer) {
	gettimeofday(timer->end, NULL);
}

int timer_get_secs(struct timer_s *timer) {
	return timer->end->tv_sec - timer->start->tv_sec;
}

void timer_refresh(struct timer_s *timer) {
	timer_start(timer);
}

void timer_reset(struct timer_s *timer) {
	timer->start->tv_sec = 0;
	timer->start->tv_usec = 0;
	timer->end->tv_sec = 0;
	timer->end->tv_usec = 0;
}

int timer_is_started(struct timer_s *timer) {
	return timer->start->tv_sec == 0 ? FALSE : TRUE;
}

void timer_free(struct timer_s *timer) {
	free(timer->start);
	free(timer->end);
	free(timer);
}
