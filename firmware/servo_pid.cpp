#include "servo_pid.h"

static void clamp(float& value,const float& min,const float& max)
{
  if(value>max)
    value=max;
  if(value<min)
    value=min;
}

servo_pid::servo_pid(const unsigned int pin,float pid_gain[3],const float min,const float max):_pin(pin),_angle_current(0.0),_angle_target(0.0),_old_time(millis()),_min(min),_max(max)
{
  _pid_gain[0]=pid_gain[0];
  _pid_gain[1]=pid_gain[1];
  _pid_gain[2]=pid_gain[2];
  
  _pid_error[0]=0.0;
  _pid_error[1]=0.0;
  _pid_error[2]=0.0;
}

void servo_pid::setup()
{
  pinMode(_pin,OUTPUT);
  _servo.attach(_pin);
}

void servo_pid::loop(const unsigned int dt)
{
  if(millis()-_old_time>dt)
  {
	_old_time=millis();
	_pid_error[1]+=_angle_target;
	clamp(_pid_error[1],-50,50);
	_pid_error[2]=_angle_target-_pid_error[0];
	_pid_error[0]=_angle_target;
	_angle_current+=(_pid_error[0]*_pid_gain[0])+(_pid_error[1]*_pid_gain[1])+(_pid_error[2]*_pid_gain[2]);
	clamp(_angle_current,1000.0,2000.0);
	_servo.writeMicroseconds((int)_angle_current);
  }
}

void servo_pid::target(const float angle)
{
  _angle_target=angle;
}
