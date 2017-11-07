#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

struct timer_s {
	struct timeval *start;
	struct timeval *end;
};

struct timer_s *timer_init();

void timer_start(struct timer_s *timer);

void timer_stop(struct timer_s *timer);

int timer_get_secs(struct timer_s *timer);

void timer_refresh(struct timer_s *timer);

void timer_reset(struct timer_s *timer);

int timer_is_started(struct timer_s *timer);

void timer_free(struct timer_s *timer);

#endif
