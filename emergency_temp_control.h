#ifndef EMERGENCY_TEMP_CONTROL_H
#define EMERGENCY_TEMP_CONTROL_H

/*
 * Entry point to thread
 * Passed tempreture_mutex defined in fan_mutex.h
 */
void * emergency_temp_control(void * args);

#endif
