#ifndef FANCONTROL_H
#define FANCONTROL_H

// Sleep interval
const int interval = 5;

// File paths for fan and tempreture readings
char * fan_file_path = "/sys/devices/pwm_fan/hwmon/hwmon0/pwm1";
char * wifi_file_path = "/sys/class/hwmon/hwmon1/temp2_input";
char * ddr_file_path = "/sys/class/hwmon/hwmon1/temp1_input";
char * cpu_file_path = "/sys/class/hwmon/hwmon2/temp1_input";

// The fan will turn on if any of these temperatures is exceeded
const int CPU_ON = 60000; // Belkin default is 85
const int DDR_ON = 60000; // Belkin default is 65
const int WIFI_ON = 65000; // Belkin default is 105

// Tempreture to begin spinning down fan from full if reached
const int CPU_THIRD_QUART = 57000;
const int DDR_THIRD_QUART = 57000;
const int WIFI_THIRD_QUART = 62000;

// The fan will run at reduced speed if any of these temperatures are below these values
const int CPU_MID = 55000; // Belkin sets no default
const int DDR_MID = 55000; // Belkin sets no default
const int WIFI_MID = 60000; // Belkin sets no default

// Begin spinning down fan from mid if reached
const int CPU_SECOND_QUART = 50000;
const int DDR_SECOND_QUART = 50000;
const int WIFI_SECOND_QUART = 55000;

// The fan will turn off if all of the temperatures are below these values
const int CPU_OFF = 45000; // Belkin defult is 80
const int DDR_OFF = 45000; // Belkin default is 60
const int WIFI_OFF = 50000; // Belkin default is 100

// Fan speeds
int target_pwm = 0; // The fan speed which is being attempted to acheive
/*
 * The current fan speed
 * Initally set to -1 untill it can be read from file
 */
int current_pwm = -1;

// Numbers for power
const double power_base = 2;
double power_exp = 1;

#endif
