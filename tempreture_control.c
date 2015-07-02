#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h> // sleep()
#include <pthread.h>

#include "fan_mutex.h"
#include "tempreture_control.h"

void * tempreture_control(void * args){

    int * cpu = ((tempreture_mutex *)args)->cpu_temp;
    int * speed = ((tempreture_mutex *)args)->target_speed; 

    pthread_mutex_t * fan_speed_write_mutex = ((tempreture_mutex*)args)->fan_speed_write_lock;
    pthread_mutex_t * fan_mutex = ((tempreture_mutex *)args)->fan_lock;
    pthread_mutex_t * tempreture_lock = ((tempreture_mutex *)args)->tempreture_lock;

    // The cpu tempreture
    int cpu_temp;
    while(1){
	// Get cpu tempreture

	pthread_mutex_lock(tempreture_lock);
	cpu_temp = *cpu;
	pthread_mutex_unlock(tempreture_lock);

	// Check lock if we cannot get it we want to skip this section
	if(pthread_mutex_trylock(fan_speed_write_mutex) == 0){
	    // Check cpu tepretures and set appropriate speed
	    if(cpu_temp >= 70000){
		pthread_mutex_lock(fan_mutex);
		*speed = 255;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 67500){
		pthread_mutex_lock(fan_mutex);
		*speed = 223;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 65000){
		pthread_mutex_lock(fan_mutex);
		*speed = 191;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 62500){
		pthread_mutex_lock(fan_mutex);
		*speed = 159;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 60000){
		pthread_mutex_lock(fan_mutex);
		*speed = 127;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 55000){
		pthread_mutex_lock(fan_mutex);
		*speed = 95;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp >= 50000){
		pthread_mutex_lock(fan_mutex);
		*speed = 63;
		pthread_mutex_unlock(fan_mutex);

	    } else if (cpu_temp < 50000){
		// Prevent fan wear
		pthread_mutex_lock(fan_mutex);
		*speed = 0;
		pthread_mutex_unlock(fan_mutex);
	    }
	    // Release lock
	    pthread_mutex_unlock(fan_speed_write_mutex);
	}
	sleep(20);
    }
}
