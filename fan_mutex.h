#ifndef FAN_MUTEX_H
#define FAN_MUTEX_H

typedef struct {
    int current_speed;
    int * target_speed;
    pthread_mutex_t * lock;
} fan_mutex;

#endif
