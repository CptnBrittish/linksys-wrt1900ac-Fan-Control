#ifndef FAN_SETTINGS_H
#define FAN_SETTINGS_H

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

#endif
