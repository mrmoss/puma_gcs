#include "gcs.h"

gcs::gcs(HardwareSerial& serial,const unsigned int baud):flags(0x01),camera_angle(0),camera_servo(0),
  pos(0.0,0.0,0.0),_serial(&serial),_baud(baud),_packet_pointer(0),_img1_seq(0),_img2_size(0)
{}

void gcs::setup()
{
  _serial->begin(_baud);
}

void gcs::loop()
{
  uint8_t read_byte;

  if(_serial->available()>0&&_serial->readBytes((char*)&read_byte,1)==1)
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
	  flags=read_byte;
	  _packet_pointer=0;
	}
	else
	  _packet_pointer=0;
  }
}

void gcs::img1_send_block(uint8_t* buffer,const short frame_size,const char block_size)
{
  _serial->write((uint8_t*)"img1",4);
  _serial->write((uint8_t*)&_img1_seq,2);
  _serial->write((uint8_t*)&frame_size,2);
  _serial->write((uint8_t*)&block_size,2);
  _serial->write(buffer,block_size);
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
