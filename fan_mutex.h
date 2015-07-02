#ifndef FAN_MUTEX_H
#define FAN_MUTEX_H

typedef struct {
    int current_speed;
    int * target_speed;
    pthread_mutex_t * lock;
} fan_mutex;

typedef struct {
    int * cpu_temp;
    int * ddr_temp;
    int * wifi_temp;
    int * target_speed; // Target fan speed
    pthread_mutex_t * fan_lock; // To prevent writing and reading of fan value
    pthread_mutex_t * fan_speed_write_lock; // to prevent writing to fan value not reading
    pthread_mutex_t * tempreture_lock;
} tempreture_mutex;

#endif
