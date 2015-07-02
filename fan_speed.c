#include <pthread.h>
#include <unistd.h> // sleep()
#include <math.h> // pow() and ceil()
#include <syslog.h>
#include <stdio.h>

#include "fan_speed.h"
#include "fan_mutex.h"

/*
 * The current fan speed
 * Initally set to -1 untill it can be read from file
 * Does not require lock as is only set once before thread in init
 */
int current_pwm = 0;

// File path for fan seed
char * fan_file_path = "/sys/devices/pwm_fan/hwmon/hwmon0/pwm1";

// Numbers for power
const double power_base = 2;
double power_exp = 1;

/* 
 * Fan needs a whole number for speed but we incrment in values of 0.1
 * Hence convert the double from pow into a intger
 * If we have a number with a decimal place round number up
 */
int power(double base, double exp) {
    double result = pow(base, exp);
    result = ceil(result);
    return (int) result;
}

/*
 * According to JimWright the fan only begins to move when set to 100
 * after being set to 100 the fan can be run at reduced speeds
 * whever the fan has been stopped first set the speed to 100 to start the fan
 * refrence: https://forum.openwrt.org/viewtopic.php?pid=278818#p278818
 */
void start_fan(){
    current_pwm = 100;

    // Notify of fan speed change
    syslog(LOG_INFO, "Starting fan");
    //Write current_pwm to device
    FILE * fan_speed_file = fopen(fan_file_path, "w");

    if(fan_speed_file != NULL){
	fprintf(fan_speed_file, "%d", current_pwm);
	fclose(fan_speed_file);
    } else {
	syslog(LOG_ERR, "Cannot open file to change fan speed");
    }
    // Bring the exponate used for the current speed to the closest possible value above the current fan speed
    power_exp = 7;
    current_pwm = 127;
}

/*
 * Sets fan speed
 * Will change fan speed incrmentally untill it reaches the speed passed to it
 * Speed is incrmented by a exponential curve based on 2^n
 * n increments by 0.1
 */
void * fan_set(void * args){
    int * target_speed = ((fan_mutex *)args)->target_speed;
    current_pwm = ((fan_mutex *)args)->current_speed;
    pthread_mutex_t * target_speed_lock = ((fan_mutex *)args)->lock;

    int target_fan_speed = 0;

    while(1){ 
	// Get fan speed
	pthread_mutex_lock(target_speed_lock);
	target_fan_speed = *target_speed;
	pthread_mutex_unlock(target_speed_lock);
	if(target_fan_speed > 0 && current_pwm == 0){
	    start_fan();
	}

	if(target_fan_speed != current_pwm){
	    // We cannot reduce fan speed to zero using powers hardcode zero option
	    if((target_fan_speed == 0) && (current_pwm == 1)){
		current_pwm = 0;
	    }
	    // Check if we need to increase fan speed
	    if(target_fan_speed > current_pwm){
		// We need to increase the exponate to create our new number
		power_exp = power_exp + 0.1;
		current_pwm = power(power_base, power_exp);
		if(target_fan_speed < current_pwm){
		    current_pwm = target_fan_speed;
		}
	    }
	    // Check if we need to decrease fan speed
	    if(target_fan_speed < current_pwm){
		power_exp = power_exp - 0.1;
		current_pwm = power(power_base, power_exp);
		if(target_fan_speed > current_pwm){
		    current_pwm = target_fan_speed;
		}
	    }


	    //Write current_pwm to device
	    FILE * fan_speed_file = fopen(fan_file_path, "w");
	    if(fan_speed_file != NULL){
		// Notify of fan speed change
		syslog(LOG_INFO, "Changing Fan speed to %i, target speed %i", current_pwm, target_fan_speed);
		fprintf(fan_speed_file, "%d", current_pwm);
		fclose(fan_speed_file);
	    } else {
		syslog(LOG_ERR, "Cannot open file to change fan speed");
	    }

	}
	sleep(1);
    }
}
