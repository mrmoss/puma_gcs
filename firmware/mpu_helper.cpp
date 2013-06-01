//MEGA ONLY

#include "mpu_helper.h"

#include <Wire.h>
#include "MPU6050_6Axis_MotionApps20.h"

static MPU6050 mpu;
static bool mpu_interrupt_flag=false;
static servo_pid* mpu_servo_controller=NULL;
static Quaternion mpu_quat;
static VectorFloat mpu_gravity;
static float mpu_orientation[3];
static uint16_t fifo_packet_size=0;
static uint8_t fifo_buffer[64];

static void mpu_interrupt_function()
{
  mpu_interrupt_flag=true;
}

static void mpu_update()
{
  TWCR|=0x04;

  uint16_t fifo_count=mpu.getFIFOCount();
  uint8_t interrupt_status=mpu.getIntStatus();

  if(fifo_count==1024||(interrupt_status&0x10))
  {
    mpu.resetFIFO();
  }

  if(interrupt_status&0x02)
  {
    while(fifo_count>=fifo_packet_size)
    {
      mpu.getFIFOBytes(fifo_buffer,fifo_packet_size);
      fifo_count-=mpu.dmpGetFIFOPacketSize();
      mpu.dmpGetQuaternion(&mpu_quat,fifo_buffer);
      mpu.dmpGetGravity(&mpu_gravity,&mpu_quat);
      mpu.dmpGetYawPitchRoll(mpu_orientation,&mpu_quat,&mpu_gravity);
      mpu_servo_controller->set_target(mpu_orientation[1]*180.0/M_PI);
    }
  }

  TWCR&=~(0x04);
}

bool mpu_setup(servo_pid& servo_controller)
{
  pinMode(53,OUTPUT);
  mpu_servo_controller=&servo_controller;
  Wire.begin();
  mpu.initialize();

  if(mpu.testConnection()&&mpu.dmpInitialize()==0)
  {
    fifo_packet_size=mpu.dmpGetFIFOPacketSize();
    mpu.setDMPEnabled(true);
    pinMode(2,INPUT);
    attachInterrupt(0,mpu_interrupt_function,RISING);
    mpu.getIntStatus();
    return true;
  }

  return false;
}

void mpu_loop()
{
  if(mpu_interrupt_flag)
  {
    mpu_interrupt_flag=false;
    mpu_update();
  }
}
