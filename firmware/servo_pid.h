#ifndef SERVO_PID_H
#define SERVO_PID_H

#include <Arduino.h>
#include <Servo.h>

class servo_pid
{
  public:
    servo_pid(const unsigned int pin,float pid_gain[3],const float min,const float max);

    void setup();
    void loop(const unsigned int dt);

    void target(const float angle);

  private:
    unsigned int _pin;
    Servo _servo;
    float _angle_current;
    float _angle_target;
    float _pid_gain[3];
    float _pid_error[3];
    long _old_time;
    float _min;
    float _max;
};

#endif
