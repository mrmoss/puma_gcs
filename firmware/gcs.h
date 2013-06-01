#ifndef GCS_H
#define GCS_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "location.h"

#include "servo_pid.h"

class gcs
{
  public:
    gcs(HardwareSerial& serial,const unsigned int baud);

    void setup(servo_pid& servo_controller);
    void loop();

    void stat_send();
    void img1_send_block(uint8_t* buffer,const short frame_size,const char block_size);
    void img2_send_pos(const location& pos);

  private:
    HardwareSerial* _serial;
    unsigned int _baud;
    unsigned int _packet_pointer;
    long _old_time;
    char _flags;
    location _pos;
    short _img1_seq;
    short _img2_size;
    servo_pid* _servo_controller;
};

#endif
