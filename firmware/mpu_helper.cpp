#include "mpu_helper.h"

#include <Wire.h>
#include "MPU6050_6Axis_MotionApps20.h"

static MPU6050 mpu;
static uint8_t mpuIntStatus;

static uint8_t devStatus;
static uint16_t packetSize;
static uint16_t fifoCount;
static uint8_t fifoBuffer[64];
static volatile bool mpu_interrupt = false;
static Quaternion mpu_quat;
static VectorFloat mpu_gravity;
static float mpu_orientation[3];
static servo_pid* mpu_servo_controller;

static void dmpDataReady()
{
  mpu_interrupt=true;
}

static void getIMU(void)
{
  fifoCount=mpu.getFIFOCount();

  mpuIntStatus = mpu.getIntStatus();
  if((mpuIntStatus&0x10)||fifoCount==1024)
  {
    mpu.resetFIFO();
  }

  else if(mpuIntStatus&0x02)
  {
    while(fifoCount>=packetSize)
    {
      mpu.getFIFOBytes(fifoBuffer,packetSize);
      fifoCount-=packetSize;

      mpu.dmpGetQuaternion(&mpu_quat,fifoBuffer);
      mpu.dmpGetGravity(&mpu_gravity,&mpu_quat);
      mpu.dmpGetYawPitchRoll(mpu_orientation,&mpu_quat,&mpu_gravity);

      mpu_servo_controller->target(mpu_orientation[1]*180.0/M_PI);
    }
  }
}

bool mpu_setup(servo_pid& servo_controller)
{
  pinMode(53,OUTPUT);
  mpu_servo_controller=&servo_controller;
  Wire.begin();

  devStatus = mpu.dmpInitialize();

  if(mpu.testConnection()&&devStatus == 0)
  {
    mpu.setDMPEnabled(true);
    pinMode(2,INPUT);
    attachInterrupt(0,dmpDataReady,RISING);
    mpuIntStatus = mpu.getIntStatus();
    packetSize = mpu.dmpGetFIFOPacketSize();
    return true;
  }

  return false;
}

void mpu_loop()
{
  if(mpu_interrupt)
  {
    mpu_interrupt=false;
    getIMU();
  }
}
