#include <pthread.h>
#include <unistd.h> // sleep()
#include <syslog.h>
#include <stdio.h>

#include "emergency_temp_control.h"
#include "fan_mutex.h"

int cpu_temp_prev;
int cpu_temp_cur;
int cpu_diff;

int ddr_temp_prev;
int ddr_temp_cur;
int ddr_diff;

int wifi_temp_prev;
int wifi_temp_cur;
int wifi_diff;

float load_one;
float load_five;
float load_fifteen;

int * cpu_temp;
int * ddr_temp;
int * wifi_temp;
int * speed;

pthread_mutex_t * tempreture_lock;
pthread_mutex_t * fan_speed_write_lock;
pthread_mutex_t * fan_lock;

void get_tempretures(){
	// get tempretures and calc their diffrence    
	pthread_mutex_lock(tempreture_lock);

	cpu_temp_prev = cpu_temp_cur;
	cpu_temp_cur = *cpu_temp;
	cpu_diff = cpu_temp_cur - cpu_temp_prev;

	ddr_temp_prev = ddr_temp_cur;
	ddr_temp_cur = *ddr_temp;
	ddr_diff = ddr_temp_cur - ddr_temp_prev;

	wifi_temp_prev = wifi_temp_cur;
	wifi_temp_cur = *wifi_temp;
	wifi_diff = wifi_temp_cur - wifi_temp_prev;

	pthread_mutex_unlock(tempreture_lock);
}

void * emergency_temp_control(void * args){
    cpu_temp = ((tempreture_mutex *)args)->cpu_temp;
    ddr_temp = ((tempreture_mutex *)args)->ddr_temp;
    wifi_temp = ((tempreture_mutex *)args)->wifi_temp;
    speed = ((tempreture_mutex *)args)->target_speed;

    tempreture_lock = ((tempreture_mutex *)args)->tempreture_lock;
    fan_speed_write_lock = ((tempreture_mutex *)args)->fan_speed_write_lock;
    fan_lock = ((tempreture_mutex *)args)->fan_lock;

    // Get current temps
    pthread_mutex_lock(tempreture_lock);
    cpu_temp_cur = *cpu_temp;
    ddr_temp_cur = *ddr_temp;
    wifi_temp_cur = *wifi_temp;
    pthread_mutex_unlock(tempreture_lock);

    while(1){
	get_tempretures();
	// Get load averages
	FILE * file = fopen("/proc/loadavg", "r");
	if(file != NULL){
	    fscanf(file, "%f %f %f", &load_one, &load_five, &load_fifteen);
	    fclose(file);
	}
	// As we have the same action for all options do one if statement
	// The compiler should optimize this
	// Only change for cpu variation if the cpu is changing by 2 degrees and over 50 degrees below that it is normal variation or to cool and getting back to normal operating tempreture
	if(((cpu_diff >= 2000) && (cpu_temp_cur >= 50000)) || (ddr_diff >= 2000) || (wifi_diff >= 2000) || (load_one >= 1.0) || (load_five >= 1.0) || (load_fifteen >= 1.0)){
	    // Hold writelock for entire operation but makeit so the fan speed fuunction can read the value
	    pthread_mutex_lock(fan_speed_write_lock);
	    pthread_mutex_lock(fan_lock);
	    *speed = 255;
	    pthread_mutex_unlock(fan_lock);
	    syslog(LOG_INFO, "Emergency Fan Speed change to 255 for 30 seconds");
	    sleep(30);
	    pthread_mutex_unlock(fan_speed_write_lock);
	    // We have been sleeping for 30 seconds so tempretures should of reduced by more then the needed value
	    // Reset tempreture changes
	    get_tempretures();
	} else {
	    sleep(5);
	}
    }
}
