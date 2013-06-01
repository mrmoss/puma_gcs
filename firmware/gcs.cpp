#include "gcs.h"

gcs::gcs(HardwareSerial& serial,const unsigned int baud):_serial(&serial),_baud(baud),_packet_pointer(0),
  _old_time(millis()),_flags(0x01),_pos(0.0,0.0,0.0),_img1_seq(0),_img2_size(0),_servo_controller(NULL)
{}

void gcs::setup(servo_pid& servo_controller)
{
  _serial->begin(_baud);
  _servo_controller=&servo_controller;
}

void gcs::loop()
{
  uint8_t read_byte;

  while(_serial->available()>0&&_serial->readBytes((char*)&read_byte,1)==1)
  {
	if(_packet_pointer==0&&read_byte=='c')
	  _packet_pointer=1;
	else if(_packet_pointer==1&&read_byte=='m')
	  _packet_pointer=2;
	else if(_packet_pointer==2&&read_byte=='n')
	  _packet_pointer=3;
	else if(_packet_pointer==3&&read_byte=='d')
	  _packet_pointer=4;
	else if(_packet_pointer==4)
	{
	  _flags=read_byte;
	  _packet_pointer=0;
	}
	else
	  _packet_pointer=0;
  }

  //if(_flags&0x01)
  {
    if(_old_time-millis()>=2000)
    {
      stat_send();
     _old_time=millis(); 
    }
  }
}

void gcs::stat_send()
{
  float camera_angle=_servo_controller->target();
  float servo_angle=_servo_controller->current();
  _serial->write((uint8_t*)"stat",4);
  _serial->write((uint8_t*)&camera_angle,4);
  _serial->write((uint8_t*)&servo_angle,4);
  _serial->write((uint8_t*)&(_pos.lat),4);
  _serial->write((uint8_t*)&(_pos.lng),4);
  _serial->write((uint8_t*)&(_pos.alt),4);
}

void gcs::img1_send_block(uint8_t* buffer,const short frame_size,const char block_size)
{
  _serial->write((uint8_t*)"img1",4);
  _serial->write((uint8_t*)&_img1_seq,2);
  _serial->write((uint8_t*)&frame_size,2);
  _serial->write((uint8_t*)&block_size,2);
  _serial->write(buffer,block_size);
  _serial->println("");
  ++_img1_seq;
}

void gcs::img2_send_pos(const location& pos)
{
  ++_img2_size;
  _serial->write((uint8_t*)"img2",4);
  _serial->write((uint8_t*)&_img2_size,2);
  _serial->write((uint8_t*)&pos.lat,4);
  _serial->write((uint8_t*)&pos.lng,4);
  _serial->write((uint8_t*)&pos.alt,4);
}
