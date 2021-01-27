
#ifndef TIMER_H
#define TIMER_H

typedef struct timer *timer;

timer timer_begin();
double timer_total_get(timer t);
void timer_print(timer t, double time_interval, const char *fmt, ...);
void timer_end(timer *t, const char *fmt, ...);

#endif
