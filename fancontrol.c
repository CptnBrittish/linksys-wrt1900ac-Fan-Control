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
#include <math.h> // pow and ceil

#include "fancontrol.h"

// As we enter whole numbers into fan speed we need to convert to a int
int power(double base, double exp) {
    double result = pow(base, exp);
    result = ceil(result);
    return (int) result;
}

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

/*
 * According to JimWright the fan onnly begins to move when set to 100
 * after being set to 100 the fan can be run at reduced speeds
 * whever the fan has been stopped first set the speed to 100 to start the fan
 * refrence: https://forum.openwrt.org/viewtopic.php?pid=278818#p278818
 */
void start_fan(){
    current_pwm = 100;

    // Notify of fan speed change
    syslog(LOG_INFO, "Starting fan");
    //Write current_pwm to device
    FILE * fan_speed = fopen(fan_file_path, "w");

    if(fan_speed != NULL){
	fprintf(fan_speed, "%d", current_pwm);
	fclose(fan_speed);
    } else {
	syslog(LOG_ERR, "Cannot open file to change fan speed");
    }
    // Bring the exponate used for the current speed to the closest possible value above the current fan speed
    power_exp = 7;
    current_pwm = 127;
}

void fan_set(int speed){
    target_pwm = speed;
 
    if(target_pwm > 0 && current_pwm == 0){
	start_fan();
    }

    if(target_pwm != current_pwm){
	while(target_pwm != current_pwm){
	    // Check if we need to increase fan speed
	    if(target_pwm > current_pwm){
		// We need to increase the exponate to create our new number
		power_exp = power_exp + 0.1;
		current_pwm = power(power_base, power_exp);
		if(target_pwm < current_pwm){
		    current_pwm = target_pwm;
		}
	    }
	    // Check if we need to decrease fan speed
	    if(target_pwm < current_pwm){
		power_exp = power_exp - 0.1;
		current_pwm = power(power_base, power_exp);
		if(target_pwm > current_pwm){
		    current_pwm = target_pwm;
		}
	    }

	    //Write current_pwm to device
	    FILE * fan_speed = fopen(fan_file_path, "w");
	    if(fan_speed != NULL){
		fprintf(fan_speed, "%d", current_pwm);
		fclose(fan_speed);
	    } else {
		syslog(LOG_ERR, "Cannot open file to change fan speed");
	    }
	    sleep(1);
	}
    }
}

int main(void){
    openlog(NULL, LOG_PID, LOG_USER);
// Get current fan speed in case it has previously been set before run time
    while(current_pwm == -1){
	current_pwm = read_int_from_file(fan_file_path); 
    }
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
	    // Notify of fan speed change
	    syslog(LOG_INFO, "Changing Fan speed to 255");	    
	    fan_set(255);

	    while((cpu > CPU_THIRD_QUART) && (ddr > DDR_THIRD_QUART) && (wifi > WIFI_THIRD_QUART)){
		//get tempreture values
		cpu = read_int_from_file(cpu_file_path);
		if(cpu == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open CPU Temperature");
		    //Set to highest possible tempreture to be safe
		    cpu = CPU_ON;
		}

		ddr = read_int_from_file(ddr_file_path);
		if(ddr == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open DDR Temperature");
		    ddr = DDR_ON;
		}

		wifi = read_int_from_file(wifi_file_path);
		if(wifi == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open Wifi Temperature");
		    wifi = WIFI_ON;
		    sleep(1);
		}
		fan_set(192);
	    }

	} else if ((cpu < CPU_OFF) && (ddr < DDR_OFF) && (wifi < WIFI_OFF)){
	    // Notify of fan speed change
	    syslog(LOG_INFO, "Turning Fan off");	    
	    fan_set(0);

	} else if (cpu < CPU_MID && ddr < DDR_MID && wifi < WIFI_MID){
	    // Notify of fan speed change
	    syslog(LOG_INFO, "Changing Fan speed to 128");
	    fan_set(128);

	    while((cpu > CPU_SECOND_QUART) && (ddr > DDR_SECOND_QUART) && (wifi > WIFI_SECOND_QUART)){
		//get tempreture values
		cpu = read_int_from_file(cpu_file_path);
		if(cpu == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open CPU Temperature");
		    //Set to highest possible tempreture to be safe
		    cpu = CPU_ON;
		}

		ddr = read_int_from_file(ddr_file_path);
		if(ddr == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open DDR Temperature");
		    ddr = DDR_ON;
		}

		wifi = read_int_from_file(wifi_file_path);
		if(wifi == -1){
		    //Complain to std error
		    syslog(LOG_ERR, "Cannot open Wifi Temperature");
		    wifi = WIFI_ON;
		}
		sleep(1);
	    }
	    fan_set(64);

	}
	sleep(interval);
    }      
}
