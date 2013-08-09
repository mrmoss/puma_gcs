#include <Servo.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#define SHOW_DEBUG
// MPU control/status vars
MPU6050 mpu;
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high



// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
//

Servo rollServo;

unsigned long time = 0; 
unsigned long lastServoTime = 0;


double calibration_angle = 6.0;
double pGain = 3.0;
double dGain = 10.0;
double iGain = 0.1;
double cameraAngleSum = 0.0;
double oldCameraAngle = 0.0;
double oldError = 0.0;
double newError = 0.0;
double dError = 0.0;
double cameraAngle = 0.0;
double servoAngle = 1500.0;
double maxServoAngle = 1600.0;
double minServoAngle = 1400.0;

unsigned long dtServoControl = 10;

void setup()
{

  Serial.begin(57600);
  rollServo.attach(6);

  Wire.begin();



  //initialize the MPU6050
  mpu.initialize();





  if(!mpu.testConnection())
  {
#ifdef SHOW_DEBUG
    Serial.println("MPU6050 Connection failed!!");
    // return;
#endif
  }
  else
  {
#ifdef SHOW_DEBUG
    Serial.println("MPU6050 Connection confirmed!!");
#endif
  }

  delay(1000);

  devStatus = mpu.dmpInitialize();
  if(devStatus == 0)
  {
#ifdef SHOW_DEBUG
    Serial.println("DMP Initialized");
#endif
    //turn on the DMP
    mpu.setDMPEnabled(true);

    //enable Interrupts
    pinMode(2, INPUT);
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

#ifdef SHOW_DEBUG
    Serial.println("DMP Ready!  Waiting for first interrupt....");
#endif

    dmpReady = true;

    packetSize = mpu.dmpGetFIFOPacketSize();
  }
  else
  {
#ifdef SHOW_DEBUG
    Serial.print("DMP Initialization failed (code ");
    Serial.print(devStatus);
    Serial.println(")");
#endif
  }

  servoAngle = 1500.0;
  rollServo.writeMicroseconds((int)servoAngle);

  delay(1000);


}


void loop()
{

  //Get IMU Data
  if(mpuInterrupt == true)
  {
    mpuInterrupt = false;
    getIMU();
  }

  //  //Command the Servo to new location
  if( ( (millis() - lastServoTime ) > dtServoControl ))  //servo control is 12.5 Hz
  {
    lastServoTime = millis();

    dtServoControl = 10;    //..................................................................set the time delay between servo updates
    maxServoAngle = 2000.0; //..................................................................max servo clamping value
    minServoAngle = 1000.0; //..................................................................min servo clamping value

    pGain = 3.0;//..............................................................................Gain variables adjusted     
    dGain = 1.0;
    iGain = 0.02;

    newError = cameraAngle; //.................................................................. proportional error

    cameraAngleSum += newError; //...............................................................integral error
    if(cameraAngleSum > 50) cameraAngleSum = 50; //..............................................clamp sum max
    else if(cameraAngleSum < -50) cameraAngleSum = -50; //.......................................clamp sum mim

    dError = newError - oldError; //.............................................................derivative error     
    oldError = newError; //......................................................................store new error as old error

      servoAngle = servoAngle + (newError * pGain) - (dError * dGain) + (cameraAngleSum*iGain); //..PID equation

      if(servoAngle <minServoAngle) servoAngle = minServoAngle;  //................................clamp the max servo value
    else if(servoAngle > maxServoAngle) servoAngle = maxServoAngle; //...........................clamp the min servo value

      rollServo.writeMicroseconds((int)servoAngle); //...............................................write command to servo

  }

}

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================


void dmpDataReady() 
{
  mpuInterrupt = true;
}

// ================================================================
// ===                 IMU DATA FUNCTION                        ===
// ================================================================

void getIMU(void)
{
  //mpu.setDMPEnabled(false);
  //int correctionAngle = 0;
  //int rollAngle = 0;
  // Serial.println("Get IMU Entered");
  //TWCR |= (0x04); //turn on I2C hardware function

  mpuIntStatus = mpu.getIntStatus(); //which interrupt occurred?
  //Serial.println(mpuIntStatus);

  fifoCount = mpu.getFIFOCount();
  //Serial.println(packetSize);

  if((mpuIntStatus & 0x10) || (fifoCount == 1024)) //if its an error interrupt
  {
    mpu.resetFIFO();
  }
  else if(mpuIntStatus & 0x02) //if its data
  {
    //fifoCount = 0;
    // while(fifoCount < packetSize) fifoCount = mpu.getFIFOCount();//wait until the correct amount of data is in the buffer.
    //fifoCount = mpu.getFIFOCount();
    while(fifoCount >= packetSize) 
    {
      mpu.getFIFOBytes(fifoBuffer, packetSize); //get data and place in buffer
      fifoCount -= packetSize;
      mpu.dmpGetQuaternion(&q, fifoBuffer); //turn the buffered data into quaternions
      mpu.dmpGetGravity(&gravity, &q);      //get gravity vector
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity); //get actual angles of IMU
      cameraAngle = (double)(ypr[2] * 180.0/M_PI);// + calibration_angle; //convert into degrees (and add calibration if required)
      // Serial.print("CA: ");
      // Serial.println(cameraAngle);
    }


  }//end if and elseif
  // Serial.println("Get IMU Exited");
  //TWCR &= ~(0x04);  //turn off I2C hardware function
}//end getIMU





