/*
  The MIT License (MIT)

  Copyright (c) 2015 Thomas Atkinson

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/*
 * Program to monitor tempreture and adjust fan speed for Linksys wrt1900ac
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h> // sleep()
#include <pthread.h>
#include <errno.h>

#include "fancontrol.h"
#include "fan_settings.h"
#include "fan_speed.h"
#include "fan_mutex.h"

pthread_mutex_t target_pwm_lock;

/* 
 * Read a positive integer from a file.
 * Treats the entire file as one number
 * Will return the integer
 * Returns -1 on error
 */
int read_int_from_file(char * file_path){
    int integer = 0;
    FILE * file = fopen(file_path, "r");
    if(file != NULL){
	fscanf(file, "%i", &integer);
	//Close the file and return integer
	fclose(file);
	return integer;
    } else {
	return -1;
    }
}

int main(void){
    openlog(NULL, LOG_PID, LOG_USER);
    
    // Set variables to pass to fan speed
    fan_mutex fan_speed_parameters;
    fan_speed_parameters.current_speed = read_int_from_file("/sys/devices/pwm_fan/hwmon/hwmon0/pwm1");
    fan_speed_parameters.target_speed = &target_pwm;
    fan_speed_parameters.lock = &target_pwm_lock;
    // As we do not have threads yet we can change the variable without getting a lock
    target_pwm = fan_speed_parameters.current_speed;

    // Fan thread id
    pthread_t id;

    // Create threads
    pthread_mutex_init(&target_pwm_lock, NULL);
    int err = pthread_create(&id, NULL, &fan_set, &fan_speed_parameters);
    if(err){
	syslog(LOG_CRIT, "Cannot create thread, exiting");
	exit(EXIT_FAILURE);
    }

    // Main loop to read temp values and change target speed
    while(1){
	//get tempreture values
	int cpu = read_int_from_file(cpu_file_path);
	if(cpu == -1){
	    //Complain to std error
	    syslog(LOG_ERR, "Cannot open CPU Temperature");
	    //Set to highest possible tempreture to be safe
	    cpu = CPU_ON;
	}

	int ddr = read_int_from_file(ddr_file_path);
	if(ddr == -1){
	    //Complain to std error
	    syslog(LOG_ERR, "Cannot open DDR Temperature");
	    ddr = DDR_ON;
	}

	int wifi = read_int_from_file(wifi_file_path);
	if(wifi == -1){
	    //Complain to std error
	    syslog(LOG_ERR, "Cannot open Wifi Temperature");
	    wifi = WIFI_ON;
	}

	if((cpu > CPU_ON) && ((ddr > DDR_ON) || (wifi > WIFI_ON))){
	    pthread_mutex_lock(&target_pwm_lock);
	    target_pwm = 255;
	    pthread_mutex_unlock(&target_pwm_lock);

	} else if ((cpu < CPU_OFF) && (ddr < DDR_OFF) && (wifi < WIFI_OFF)){
	    pthread_mutex_lock(&target_pwm_lock);
	    target_pwm = 0;
	    pthread_mutex_unlock(&target_pwm_lock);

	} else if (cpu < CPU_MID && ddr < DDR_MID && wifi < WIFI_MID){
	    pthread_mutex_lock(&target_pwm_lock);
	    target_pwm = 128;
	    pthread_mutex_unlock(&target_pwm_lock);

	}
	sleep(interval);
    }      
}
