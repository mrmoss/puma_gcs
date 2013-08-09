/*************************************************** 
 * This is a library for the Adafruit TTL JPEG Camera (VC0706 chipset)
 * 
 * Pick one up today in the adafruit shop!
 * ------> http://www.adafruit.com/products/397
 * 
 * These displays use Serial to communicate, 2 pins are required to interface
 * 
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing 
 * products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 * BSD license, all text above must be included in any redistribution
 ****************************************************/


#include "VC0706.h"

// Initialization code used by all constructor types
void VC0706::common_init(void) {
  // swSerial  = NULL;
  hwSerial  = NULL;
  frameptr  = 0;
  bufferLen = 0;
  serialNum = 0;
}

// Constructor when using SoftwareSerial or NewSoftSerial
//#if ARDUINO >= 100
//VC0706::VC0706(SoftwareSerial *ser) {
//#else
//VC0706::VC0706(NewSoftSerial *ser) {
//#endif
//  common_init();  // Set everything to common state, then...
//swSerial = ser; // ...override swSerial with value passed.
//}

// Constructor when using HardwareSerial
VC0706::VC0706(HardwareSerial *ser) {
  common_init();  // Set everything to common state, then...
  hwSerial = ser; // ...override hwSerial with value passed.
}

boolean VC0706::begin(uint16_t baud) {
  Serial2.begin(baud);
  return reset();
}

boolean VC0706::reset() {
  uint8_t args[] = { 0x0  };
  boolean test = runCommand(VC0706_RESET, args, 1, 5, true);
  return test;
}



uint8_t VC0706::getImageSize() {
  uint8_t args[] = {0x4, 0x4, 0x1, 0x00, 0x19  };
//  Serial.println("getImageSize Function Called");
  if (!runCommand(VC0706_READ_DATA, args, sizeof(args), 6))
  {
    Serial.print("Error in getImageSize");
    return -1;
  }  
  Serial.print("image size read.  Value is: ");
  Serial.println(camerabuff[5]);

  return camerabuff[5];
}

boolean VC0706::setImageSize(uint8_t x) {
  uint8_t args[] = {0x05, 0x04, 0x01, 0x00, 0x19, x  };
  //Serial.println("Camera Set Image Size Called");
  bool temp = runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
  //Serial.print("SetImage worked?: ");
 // Serial.println(temp);
  return temp;
}

boolean VC0706::setBaudRate(uint8_t x) {

  uint8_t serialNum = 0;
  uint8_t args[4];
  args[0] = 0x03;
  args[1] = 0x01;

  switch(x)
  {
  case VC0706_9600_BAUD:
    args[2] = 0xAE;
    args[3] = 0xC8;
    break;
  case VC0706_19200_BAUD:
    args[2] = 0x56;
    args[3] = 0xE4;
    break;
  case VC0706_38400_BAUD:
    args[2] = 0x2A;
    args[3] = 0xF2;
    break;
  case VC0706_57600_BAUD:
    args[2] = 0x1C;
    args[3] = 0x1C;
    break;
  case VC0706_115200_BAUD:
    args[2] = 0x0D;
    args[3] = 0xA6;
    break;
  default: 
    break;
  }

  //readResponse(100, 10); 
  Serial.println("Start Baud Change");

  Serial2.write(0x56);
  Serial2.write(serialNum);
  Serial2.write(0x24);
  Serial2.write(0x03);
  Serial2.write(0x01);
  Serial2.write(0x0D);
  Serial2.write(0xA6);

  if (readResponse(5, 200)!= 5) 
    return false;
  if (! verifyResponse(0x24))
    return false;
  return true;


  // return runCommand(VC0706_CHANGE_BAUD, args, sizeof(args), 5);
}

/****************** downsize image control */

uint8_t VC0706::getDownsize(void) {
  uint8_t args[] = {
    0x0  };
  if (! runCommand(VC0706_DOWNSIZE_STATUS, args, 1, 6)) 
    return -1;

  return camerabuff[5];
}

boolean VC0706::setDownsize(uint8_t newsize) {
  uint8_t args[] = {
    0x01, newsize  };

  return runCommand(VC0706_DOWNSIZE_CTRL, args, 2, 5);
}

/***************** other high level commands */

char * VC0706::getVersion(void) {
  uint8_t args[] = {
    0x01  };

  sendCommand(VC0706_GEN_VERSION, args, 1);
  // get reply
  if (!readResponse(CAMERABUFFSIZ, 200)) 
    return 0;
  camerabuff[bufferLen] = 0;  // end it!
  return (char *)camerabuff;  // return it!
}


/****************** high level photo comamnds */

void VC0706::OSD(uint8_t x, uint8_t y, char *str) {
  if (strlen(str) > 14) { 
    str[13] = 0; 
  }

  uint8_t args[17] = {
    strlen(str), strlen(str)-1, (y & 0xF) | ((x & 0x3) << 4)  };

  for (uint8_t i=0; i<strlen(str); i++) {
    char c = str[i];
    if ((c >= '0') && (c <= '9')) {
      str[i] -= '0';
    } 
    else if ((c >= 'A') && (c <= 'Z')) {
      str[i] -= 'A';
      str[i] += 10;
    } 
    else if ((c >= 'a') && (c <= 'z')) {
      str[i] -= 'a';
      str[i] += 36;
    }

    args[3+i] = str[i];
  }

  runCommand(VC0706_OSD_ADD_CHAR, args, strlen(str)+3, 5);
  printBuff();
}

boolean VC0706::setCompression(uint8_t c) {
  uint8_t args[] = {
    0x5, 0x1, 0x1, 0x12, 0x04, c  };
  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}

uint8_t VC0706::getCompression(void) {
  uint8_t args[] = {
    0x4, 0x1, 0x1, 0x12, 0x04  };
  runCommand(VC0706_READ_DATA, args, sizeof(args), 6);
  printBuff();
  return camerabuff[5];
}

boolean VC0706::setPTZ(uint16_t wz, uint16_t hz, uint16_t pan, uint16_t tilt) {
  uint8_t args[] = {
    0x08, wz >> 8, wz, 
    hz >> 8, wz, 
    pan>>8, pan, 
    tilt>>8, tilt  };

  return (! runCommand(VC0706_SET_ZOOM, args, sizeof(args), 5));
}


boolean VC0706::getPTZ(uint16_t &w, uint16_t &h, uint16_t &wz, uint16_t &hz, uint16_t &pan, uint16_t &tilt) {
  uint8_t args[] = {
    0x0  };

  if (! runCommand(VC0706_GET_ZOOM, args, sizeof(args), 16))
    return false;
  printBuff();

  w = camerabuff[5];
  w <<= 8;
  w |= camerabuff[6];

  h = camerabuff[7];
  h <<= 8;
  h |= camerabuff[8];

  wz = camerabuff[9];
  wz <<= 8;
  wz |= camerabuff[10];

  hz = camerabuff[11];
  hz <<= 8;
  hz |= camerabuff[12];

  pan = camerabuff[13];
  pan <<= 8;
  pan |= camerabuff[14];

  tilt = camerabuff[15];
  tilt <<= 8;
  tilt |= camerabuff[16];

  return true;
}


boolean VC0706::takePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME); 
}

boolean VC0706::resumeVideo() {
  return cameraFrameBuffCtrl(VC0706_RESUMEFRAME); 
}

boolean VC0706::TVon() {
  uint8_t args[] = {
    0x1, 0x1  };
  return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}
boolean VC0706::TVoff() {
  uint8_t args[] = {
    0x1, 0x0  };
  return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}

boolean VC0706::cameraFrameBuffCtrl(uint8_t command) {
  uint8_t args[] = {
    0x1, command  };
  return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5);
}

uint32_t VC0706::frameLength(void) {
  uint8_t args[] = {
    0x01, 0x00  };
  if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9))
    return 0;

  uint32_t len;
  len = camerabuff[5];
  len <<= 8;
  len |= camerabuff[6];
  len <<= 8;
  len |= camerabuff[7];
  len <<= 8;
  len |= camerabuff[8];

  return len;
}


uint8_t VC0706::available(void) {
  return bufferLen;
}


uint8_t * VC0706::readPicture(uint8_t n) {
  uint8_t args[] = {
    0x0C, 0x0, 0x0A, 
    0, 0, frameptr >> 8, frameptr & 0xFF, 
    0, 0, 0, n, 
    CAMERADELAY >> 8, CAMERADELAY & 0xFF  };

  if (! runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false))
    return 0;


  // read into the buffer PACKETLEN!
  if (readResponse(n+5, CAMERADELAY) == 0) 
    return 0;


  frameptr += n;

  return camerabuff;
}

//************************************************************************************************//
//                           LOW LEVEL COMMANDS                                                   //
//************************************************************************************************//



boolean VC0706::runCommand(uint8_t cmd, uint8_t *args, uint8_t argn, 
uint8_t resplen, boolean flushflag) {

  // flush out anything in the buffer?
  if (flushflag) {
    readResponse(100, 10); 
  }
//  Serial.print("SerialNum: ");
//  Serial.println(serialNum);
//  Serial.print("command: ");
//  Serial.println(cmd);
//  Serial.print("argn: ");
//  Serial.println(argn);
//  Serial.print("args: ");
//  for (int ii = 0; ii < argn; ii++)
//  {
//    Serial.print(args[ii]);
//    Serial.print(",");
//  }
//  Serial.println("\n");
  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200)!= resplen) 
  {
    Serial.println("Response not the right length or too late");
    return false;
  }
  if (! verifyResponse(cmd))
  {
    return false;
  }
  return true;
}

void VC0706::sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0)
{
  Serial2.write(0x56);
  Serial2.write(serialNum);
  Serial2.write(cmd);

  for (uint8_t i=0; i<argn; i++) 
  {
    Serial2.write(args[i]);
  }
}

uint8_t VC0706::readResponse(uint8_t numbytes, uint8_t timeout) {
  uint8_t counter = 0;
  bufferLen = 0;
  int avail;

  for(int i = 0; i < 100; i++)
  {
    camerabuff[i] = 0;
  }

  while ((timeout != counter) && (bufferLen !=  numbytes)){
    //avail = swSerial ? swSerial->available() : 
    avail = Serial2.available();
    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!
    //camerabuff[bufferLen++] = swSerial ? swSerial->read() : Serial2.read();
    camerabuff[bufferLen++] = Serial2.read();
   
  }

  // Serial.print("Buffer length: ");
  //  Serial.println(bufferLen);

  // Serial.println("Camera Buffer after read: ");
  // printBuff();

  return bufferLen;
}

boolean VC0706::verifyResponse(uint8_t command) {
//  Serial.print("serialNum: ");
//  Serial.println(serialNum);
//  Serial.print("CameraBuff: ");
//  Serial.print(camerabuff[0]);
//  Serial.print(",");
//  Serial.print(camerabuff[1]);
//  Serial.print(",");
//  Serial.print(camerabuff[2]);
//  Serial.print(",");
//  Serial.println(camerabuff[3]);
  if ((camerabuff[0] != 0x76) ||
    (camerabuff[1] != serialNum) ||
    (camerabuff[2] != command) ||
    (camerabuff[3] != 0x0))
    return false;
  return true;

}

void VC0706::printBuff() {
  for (uint8_t i = 0; i< bufferLen; i++) {
    Serial.print(" 0x");
    Serial.print(camerabuff[i], HEX); 
  }
  Serial.println();
}



//*************************************************************************************
//      Motion Detection Functions
//*************************************************************************************


boolean VC0706::motionDetected() {
  if (readResponse(4, 200) != 4) {
    return false;
  }
  if (! verifyResponse(VC0706_COMM_MOTION_DETECTED))
    return false;

  return true;
}


uint8_t VC0706::setMotionStatus(uint8_t x, uint8_t d1, uint8_t d2) {
  uint8_t args[] = {
    0x03, x, d1, d2  };

  return runCommand(VC0706_MOTION_CTRL, args, sizeof(args), 5);
}


uint8_t VC0706::getMotionStatus(uint8_t x) {
  uint8_t args[] = {
    0x01, x  };

  return runCommand(VC0706_MOTION_STATUS, args, sizeof(args), 5);
}


boolean VC0706::setMotionDetect(boolean flag) {
  if (! setMotionStatus(VC0706_MOTIONCONTROL, 
  VC0706_UARTMOTION, VC0706_ACTIVATEMOTION))
    return false;

  uint8_t args[] = {
    0x01, flag  };

  runCommand(VC0706_COMM_MOTION_CTRL, args, sizeof(args), 5);
}



boolean VC0706::getMotionDetect(void) {
  uint8_t args[] = {
    0x0  };

  if (! runCommand(VC0706_COMM_MOTION_STATUS, args, 1, 6))
    return false;

  return camerabuff[5];
}

