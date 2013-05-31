//Arduino Bug...headers in include folder of compiler aren't included...
#include <Servo.h>
#include <Wire.h>

//GCS Header
#include "gcs.h"

//MPU Helper Header
#include "mpu_helper.h"

//Servo PID Header
#include "servo_pid.h"

//Global GCS Variable
gcs ground_control_station(Serial1,57600);

//Global Servo PID Controller Variables
float servo_controller_gains[3]={-2.5,0.02,-1.0};
servo_pid servo_controller(46,servo_controller_gains,1000.0,2000.0);

void setup()
{
  //Setup Debug Serial Port
  Serial.begin(57600);
  Serial.println("Starting Puma Payload");

  //Hack For Current Payload Wiring
  pinMode(47,INPUT);
  pinMode(48,INPUT);

  //Start Servo Controller
  Serial.print("servo.............");
  servo_controller.setup();
  servo_controller.target(1500);
  Serial.println("passed");

  //Start GCS
  Serial.print("gcs...............");
  ground_control_station.setup();
  Serial.println("passed");

  //Start MPU
  Serial.print("mpu...............");

  if(mpu_setup(servo_controller))
    Serial.println("passed");
  else
    Serial.println("failed");

  //All Done
  Serial.print("system g2g");
}

void loop()
{
  //Update GCS
  ground_control_station.loop();

  //Update Servo Controller
  servo_controller.loop(5);

  //Update MPU
  mpu_loop();
}
