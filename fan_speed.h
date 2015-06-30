#ifndef FAN_SPEED_H
#define FAN_SPEED_H

/*
 * Entry point to thread
 * Passed fan_mutex defined in fan_mutex.h
 */
void * fan_set(void * args);

#endif
