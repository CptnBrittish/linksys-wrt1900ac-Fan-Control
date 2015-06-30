#ifndef FANCONTROL_H
#define FANCONTROL_H

// Sleep interval
const int interval = 5;

// File paths for tempreture readings
char * wifi_file_path = "/sys/class/hwmon/hwmon1/temp2_input";
char * ddr_file_path = "/sys/class/hwmon/hwmon1/temp1_input";
char * cpu_file_path = "/sys/class/hwmon/hwmon2/temp1_input";

/*
 * Target fan speed
 * Passed to fan_speed as pointer
 */
int target_pwm = 0; // The fan speed which is being attempted to acheive

#endif
