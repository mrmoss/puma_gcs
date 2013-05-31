#ifndef MPU_HELPER_H
#define MPU_HELPER_H

//Servo PID Header
#include "servo_pid.h"

bool mpu_setup(servo_pid& servo_controller);
void mpu_loop();

#endif
