/*
 * timer.h
 *
 *  Created on: 29.12.2014
 *      Author: Daniel
 */

#ifndef LIBRARY_INC_SYS_TIMER_H_
#define LIBRARY_INC_SYS_TIMER_H_

#define TIMER_IOCTL_CONFIG 100
#define TIMER_IOCTL_RESET  101

typedef struct {
    /* period of the timer in microseconds. */
    unint4  period_us;
    /* Thread to activate on timer tick. If 0 the current thread is used. */
    unint4  threadId;
    /* priority of this timer (valid range 0-7) */
    unint1  priority;
} timer_t;

/* Method to be called by the thread upon execution finishing. Sets the thread to blocked mode
 * and waits for the next timer tick to occur again.*/
void timer_wait();

/* Configures a timer. */
int timer_configure(int fd, timer_t* timer_conf);

/* Resets the timer device. Removes the configuration (period, thread).*/
int timer_reset(int fd);

#endif /* LIBRARY_INC_SYS_TIMER_H_ */
