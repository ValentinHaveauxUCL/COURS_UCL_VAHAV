#ifndef UTILS_H
#define UTILS_H

#include "timer.h"

#define TRUE 			1
#define FALSE 			0

/* this function prints an error message and exits the program */
void exit_error(char *msg);

int is_in_interval(int a, int b, int val);

int how_many_in_interval(int a, int b, int size);

float eval_speed(long x, struct timer_s *timer);

#endif
