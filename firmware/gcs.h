#ifndef GCS_H
#define GCS_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "location.h"

class gcs
{
  public:
    gcs(HardwareSerial& serial,const unsigned int baud);

    void setup();
    void loop();

    void img1_send_block(uint8_t* buffer,const short frame_size,const char block_size);
    void img2_send_pos(const location& pos);

    char flags;
    float camera_angle;
    float camera_servo;
    location pos;

  private:
    HardwareSerial* _serial;
    unsigned int _baud;
    unsigned int _packet_pointer;
    short _img1_seq;
    short _img2_size;
};

#endif
