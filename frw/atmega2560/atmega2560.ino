#include <SD.h>
#include "VC0706.h"
#include "gps_node.h"
#include <new.h>
#include <pnew.cpp>
#include <iterator>
#include <string>
#include <algorithm>
#include "string_util.h"
#include <avr/wdt.h>




char read_byte;
int packet_state=0;
std::string packet_buffer="";
char calculate_crc(const std::string& packet)
{
  char calc=0x00;

  for(unsigned int ii=0;ii<packet.size();++ii)
    calc^=packet[ii];

  return calc;
}





#define SHOW_DEBUG
#define SHOW_HEARTBEAT_NUM
//#define SHOW_HEARTBEAT
#define SHOW_CMD_RECEIVED
#define SHOW_NEX7_DAT_1
//#define SHOW_NEX7_DAT_2
//#define SHOW_JPEG_DAT
//#define SHOW_GPS_DAT

//#define SEND_ACK


#define HBID     0
#define RADIOID  1
#define JPEGID   2
#define NEXID    3

#define BUFFERSIZE 100
#define GPSBUFFERSIZE  1000

#define PICdt  1000
#define HEARTBEATdt 1000
#define MAXPICdt  3000

gps_node gps;


const int chipSelect = 53;

char radioStatus[3];
char JPEGStatus[3];
char NEXStatus[3];
char SDStatus = 1;

char NEXBuffer[18];

char commArray[300];
char commCRC = 0;
char commNumBytesHB = 0;
char commNumBytes = 0;
char commHWID = 0;
char commIndex = 0;
char commData[50];
char commState = 0;
char inByte = 0;

char filename[13];

//time variables
unsigned long time;
unsigned long oldPICTime = 0;
unsigned long oldHeartbeatTime = 0;
unsigned long lastPictureTakenTime = 0;

char tempGPS = 0;
std::string GPSBuffer="";

int freq = 40;
int LEDPin = 12;
int LEDIntPin = 3;
int pictureNum = 0;

bool pictureTakenFlag = false;

char systemStatus = 0x00;
char commandedState = 0x01;
int heartBeatNum = 0;

/*VC0706 cam = VC0706(&Serial2);
 uint16_t jpglen=0;
 uint16_t JPEGtotalLength = 0;
 uint16_t JPEGsequenceNum = 0;
 int picture_stage=0;*/
//Camera Variables
//VC0706 jpeg_camera(&Serial1);
//bool jpeg_taken_picture=false;
//short jpeg_total_size=0;
//short jpeg_seq=0;


// ================================================================
// ===                      SETUP                               ===
// ================================================================

void setup()
{

  // wdt_enable(WDTO_8S);
  // wdt_reset();

  Serial.begin(57600);   //talking to the computer
  Serial3.begin(57600);  //talking to the GPS
  Serial2.begin(57600);  //talking to the XBEE

  pinMode(chipSelect, OUTPUT);

  SDStatus = 1;
  NEXStatus[1] = 0;
  //  JPEGStatus[1] = 0;

  pinMode(LEDPin, OUTPUT); //pin used for IR LED for camera triggering
  pinMode(LEDIntPin, INPUT); //pin used to determine whether the camera fired
  attachInterrupt(1, CameraTriggered, RISING);

#ifdef SHOW_DEBUG
  Serial.println("Initializing the SD Card...");
#endif

  if(!SD.begin(chipSelect))
  {
    SDStatus = 0; //card flag error
#ifdef SHOW_DEBUG
    Serial.println("Card failed, or not present");
    delay(100);
#endif
  }
  else
  {
#ifdef SHOW_DEBUG
    Serial.println("SD Card Initialized!  Creating File");
#endif
    strcpy(filename, "FLT_000.TXT");
    for(int i = 0; i < 1000; i++)
    {
      filename[4] = '0' + i/100;
      filename[5] = '0' + (i%100)/10;
      filename[6] = '0' + (i%100)%10;
      if(!SD.exists(filename))
      {
        break;
      }
    }
    File dataFile = SD.open(filename, FILE_WRITE);
    dataFile.close();

    if(SD.exists(filename))
    {
#ifdef SHOW_DEBUG
      Serial.print("File successfully created on card!  File name is: ");
      Serial.println(filename);
#endif
    }
    else
    {
      SDStatus = 0;
#ifdef SHOW_DEBUG
      Serial.println("File not created, please fix the problem and try again...");
      // return;
#endif
    }
  }

  //  wdt_reset();

  for(int ii = 0; ii < GPSBUFFERSIZE; ii++)
  {
    GPSBuffer[ii] = 0;
  }

  //  wdt_reset();
  /*
   // Try to locate the camera
   if(!cam.begin(38400))
   {
   #ifdef SHOW_JPEG_DAT
   Serial.println("JPEG Camera Failed");
   #endif
   JPEGStatus[1] = 0;
   //return;
   }
   else
   {
   #ifdef SHOW_JPEG_DAT
   Serial.println("JPEG Camera Found");
   #endif
   }
   
   char *reply = cam.getVersion();
   
   if (reply == 0) 
   {
   #ifdef SHOW_JPEG_DAT
   //Serial.print("Failed to get JPEG Camera Version");
   #endif
   JPEGStatus[1] = 0;
   //return;
   } 
   else
   {
   #ifdef SHOW_JPEG_DAT
   //Serial.println("-----------------");
   //Serial.print(reply);
   //Serial.println("-----------------");
   #endif
   
   }
   
   //    cam.setImageSize(VC0706_640x480);        // biggest
   //    cam.setImageSize(VC0706_320x240);        // medium
   cam.setImageSize(VC0706_160x120);          // small
   
   // You can read the size back from the camera (optional, but maybe useful?)
   #ifdef SHOW_JPEG_DAT
   uint8_t imgsize = cam.getImageSize();
   
   Serial.print("Image size: ");
   if (imgsize == VC0706_640x480) //Serial.println("640x480");
   if (imgsize == VC0706_320x240) //Serial.println("320x240");
   if (imgsize == VC0706_160x120) //Serial.println("160x120");
   #endif 
   */
  //Camera Setup
  //   if(jpeg_camera.begin(38400))
  //   {
  //   Serial.println("Camera :)");
  //   jpeg_camera.setImageSize(VC0706_160x120);
  //   }
  //   else
  //   {
  //   Serial.println("Camera :(");
  //   }
  //   
  File dataFile = SD.open(filename, FILE_WRITE);
  dataFile.println("System Status is OK");
  dataFile.close();

  Serial.println("End Setup");

  radioStatus[0] = 0;
  NEXStatus[0] = 0;
  JPEGStatus[0] = 0;


  //  wdt_enable(WDTO_2S);

}

unsigned long oldintheartbeat = 0;
// ================================================================
// ===                      LOOP                                ===
// ================================================================
void loop()
{
  // wdt_reset();
  time = millis(); //read current time

  if( (millis() - oldintheartbeat ) > 1000)
  {
    oldintheartbeat = millis();
    Serial.println("Internal heartbeat");
    //Serial.print("NEX Status: ");
    //Serial.println((int)NEXStatus[1]);
  }
  //send the heartbeat packet 
  if( ( (millis() - oldHeartbeatTime ) > HEARTBEATdt ) && (radioStatus[0] == 1) )
  {

#ifdef SHOW_HEARTBEAT_NUM
    Serial.print("heartbeat ");
    Serial.println(++heartBeatNum);
#endif
    oldHeartbeatTime = millis();
    HeartBeat();
  }

  //Take Picture
  if( ( ( millis() - oldPICTime ) > PICdt ) && (NEXStatus[0] == 1))
  {

    oldPICTime = millis();
    //pictureTakenFlag = true;
    shutterNow();
    Serial.println("picture Commanded");

  }

  while(Serial2.available()>0&&Serial2.readBytes(&read_byte,1)==1)
  {
    //Get Data Packet
    if(packet_state==0&&read_byte=='d')
    {
      packet_buffer+=read_byte;
      packet_state=1;
    }
    else if(packet_state==1&&read_byte=='a')
    {
      packet_buffer+=read_byte;
      packet_state=2;
    }
    else if(packet_state==2&&read_byte=='t')
    {
      packet_buffer+=read_byte;
      packet_state=3;
    }
    else if(packet_state==3||packet_state==4)
    {
      packet_buffer+=read_byte;
      ++packet_state;
    }
    else if(packet_state==5)
    {
      packet_buffer+=read_byte;

      if(packet_buffer.size()==static_cast<unsigned int>(packet_buffer[4])+5)
        packet_state=10;
    }

    //Get Response Packet
    else if(packet_state==0&&read_byte=='r')
    {
      packet_buffer+=read_byte;
      packet_state=6;
    }
    else if(packet_state==6&&read_byte=='e')
    {
      packet_buffer+=read_byte;
      packet_state=7;
    }
    else if(packet_state==7&&read_byte=='s')
    {
      packet_buffer+=read_byte;
      packet_state=8;
    }
    else if(packet_state==8)
    {
      packet_buffer+=read_byte;
      packet_state=9;
    }
    else if(packet_state==9)
    {
      packet_buffer+=read_byte;

      if(packet_buffer.size()==6)
        packet_state=10;
    }

    //CRC Check
    else if(packet_state==10)
    {
      if(read_byte==calculate_crc(packet_buffer))
      {
        if(msl::starts_with(packet_buffer,"dat"))
        {
          if(packet_buffer[3]==0x01)
          {
            //Read Command
            radioStatus[0]=packet_buffer[5];

            //Send Response
#ifdef SEND_ACK
            std::string response="res";
            response+=0x01;
            response+=radioStatus[0];
            response+=radioStatus[1];
            response+=calculate_crc(response);
            Serial2.write((uint8_t*)response.c_str(),response.size());
#endif
            //Debug
            //Serial.print("radio ");
            //   if(packet_buffer[5]==0x00)
            //Serial.println("off");
            //   else
            //Serial.println("on");
          }
          else if(packet_buffer[3]==0x02)
          {
            //Read Command
            JPEGStatus[0]=packet_buffer[5];

            //Send Response
#ifdef SEND_ACK
            std::string response="res";
            response+=0x02;
            response+=JPEGStatus[0];
            response+=JPEGStatus[1];
            response+=calculate_crc(response);
            Serial2.write((uint8_t*)response.c_str(),response.size());
#endif
            //Debug
            //Serial.print("jpg ");
            //    if(packet_buffer[5]==0x00)
            //Serial.println("off");
            //    else
            //Serial.println("on");
          }
          else if(packet_buffer[3]==0x03)
          {
            //Read Command
            NEXStatus[0]=packet_buffer[5];

            //Send Response
#ifdef SEND_ACK
            std::string response="res";
            response+=0x03;
            response+=NEXStatus[0];
            response+=NEXStatus[1];
            response+=calculate_crc(response);
            Serial2.write((uint8_t*)response.c_str(),response.size());
#endif
            //Debug
            //Serial.print("nex ");
            //    if(packet_buffer[5]==0x00)
            //Serial.println("off");
            //`   else
            //Serial.println("on");
          }
          else
          {
            //Serial.println("unknown packet id");
          }
        }
        else if(msl::starts_with(packet_buffer,"res"))
        {
          //Serial.println("response packet");
        }
        else
        {
          //Serial.println("unknown packet header");
        }
      }
      else
      {
        //Serial.println("bad packet");
      }

      packet_state=0;
      packet_buffer="";
    }

    //Garbage
    else
    {
      packet_state=0;
      packet_buffer="";
    }
  }

  //  wdt_reset();


  /*signed int avail = Serial2.available();
   //Get data from XBEE (new command state)
   if(avail > 0 && Serial2.readBytes(&inByte,1)==1)
   {
   switch(commState)
   {
   case 0:
   if(inByte == 'd')
   {
   commState = 1;
   commCRC = inByte;
   }
   else
   {
   commState = 0;
   commCRC = 0;
   }
   break;
   case 1:
   if(inByte == 'a')
   {
   commState = 2;
   commCRC ^= inByte;
   }
   else 
   {
   commState = 0;
   commCRC = 0;
   }
   break;
   case 2:
   if(inByte == 't')
   {
   commState = 3;
   commCRC ^= inByte;
   }
   else
   {
   commState = 0;
   commCRC = 0;
   }
   break;
   case 3:
   
   if(inByte == RADIOID || inByte == JPEGID || inByte == NEXID)
   {
   commHWID = inByte;
   commCRC ^= inByte;
   commState = 4;
   }
   else
   {
   commState = 0;
   commCRC = 0;
   }
   break;
   case 4:  
   commNumBytes = inByte;
   commCRC ^= inByte;
   commIndex = 0;
   for(int i = 0; i < 50; i++)
   {
   commData[i] = 0;
   }
   commState = 5;
   break;
   case 5:
   if(commIndex < commNumBytes)
   {
   commData[commIndex] = inByte;
   commCRC ^= inByte;
   ++commIndex; 
   }
   else
   {
   commState = 0;
   if(commCRC == inByte)
   {
   
   commCRC = 0;
   // Serial2.print("res");
   commCRC = 100;
   // Serial2.print(commHWID);
   commCRC ^= commHWID;
   switch(commHWID)
   {
   case RADIOID:
   radioStatus[0] = commData[0];
   // Serial2.print(radioStatus[0]);
   commCRC ^= radioStatus[0];
   // Serial2.print(radioStatus[1]);
   commCRC ^= radioStatus[1];
   if(radioStatus[0] == 1)
   {
   //Serial.println("Turn Radio on");
   }
   else
   {
   //Serial.println("Turn Radio off");
   }
   break;
   case JPEGID:
   JPEGStatus[0] = commData[0];
   // Serial2.print(JPEGStatus[0]);
   commCRC ^= JPEGStatus[0];
   //  Serial2.print(JPEGStatus[1]);
   commCRC ^= JPEGStatus[1];
   if(JPEGStatus[0] == 1)
   {
   //Serial.println("Turn JPEG On");
   }
   else
   {
   //Serial.println("Turn JPEG off");
   }
   break;
   case NEXID:
   NEXStatus[0] = commData[0];
   // Serial2.print(NEXStatus[0]);
   commCRC ^= NEXStatus[0];
   //  Serial2.print(NEXStatus[1]);
   commCRC ^= NEXStatus[1];
   if(NEXStatus[0] == 1)
   {
   //Serial.println("Turn NEX On");
   }
   else
   {
   //Serial.println("Turn NEX off");
   }
   break;
   default: 
   break;
   }
   // Serial2.print(commCRC);
   }
   }
   break;
   default: 
   commState = 0;
   break;
   }
   
   }*/


  //notify GCS that camera is not taking pictures, either because it is commanded to stop or because it is broken
  if( ( ( (millis() - lastPictureTakenTime ) > MAXPICdt ) && (NEXStatus[0] == 1)) || (NEXStatus[0] == 0))
  {

    NEXStatus[1] = 0;

  }

  //once the picture is taken, store the picture Number and the camera and airplane angle on the SD Card
  if(pictureTakenFlag == true)
  {
    pictureTakenFlag = false;
    ++pictureNum;
    NEXStatus[1] = 1;
    lastPictureTakenTime = millis();

    File dataFile = SD.open(filename, FILE_WRITE);
    dataFile.print(pictureNum);
    dataFile.print(',');
    dataFile.print(gps.time);
    dataFile.print(',');
    dataFile.print(gps.lat);
    dataFile.print(',');
    dataFile.print(gps.lng);
    dataFile.print(',');
    dataFile.print(gps.alt);
    dataFile.print(',');
    dataFile.print(gps.fix);
    dataFile.print(',');
    dataFile.print(gps.sat_num);
    dataFile.print(',');
    dataFile.print(gps.speed);
    dataFile.print(',');
    dataFile.print(gps.course);
    dataFile.println('\n');

    dataFile.close();

    if(radioStatus[0] == 1)
    {
      NEXBuffer[0] = 'd';
      NEXBuffer[1] = 'a';
      NEXBuffer[2] = 't';
      NEXBuffer[3] = NEXID;
      NEXBuffer[4] = 14;
      NEXBuffer[5] = ((char*)&pictureNum)[0];
      NEXBuffer[6] = ((char*)&pictureNum)[1];
      NEXBuffer[7] = ((char*)&gps.lat)[0];
      NEXBuffer[8] = ((char*)&gps.lat)[1];
      NEXBuffer[9] = ((char*)&gps.lat)[2];
      NEXBuffer[10] = ((char*)&gps.lat)[3];
      NEXBuffer[11] = ((char*)&gps.lng)[0];
      NEXBuffer[12] = ((char*)&gps.lng)[1];
      NEXBuffer[13] = ((char*)&gps.lng)[2];
      NEXBuffer[14] = ((char*)&gps.lng)[3];
      NEXBuffer[15] = ((char*)&gps.alt)[0];
      NEXBuffer[16] = ((char*)&gps.alt)[1];
      NEXBuffer[17] = ((char*)&gps.alt)[2];
      NEXBuffer[18] = ((char*)&gps.alt)[3];

      NEXBuffer[19] = 0;
      for(int ii = 0; ii < 19; ii++)
      {
        NEXBuffer[19] ^= NEXBuffer[ii];
      }

      Serial2.write( (uint8_t*)&NEXBuffer, 20);
    }

#ifdef SHOW_NEX7_DAT_1
    Serial.print("img2: ");
    Serial.println(pictureNum); 
#endif

#ifdef SHOW_NEX7_DAT_2
    Serial.print("Lat: ");
    Serial.println(PICLat);
    Serial.print("Lng: ");
    Serial.println(PICLng);
    Serial.print("Alt: ");
    Serial.println(PICAlt);
    Serial.println("\n");
#endif

  }

  if(Serial3.available() > 0 )
  {
    char tempbyte;
    while(Serial3.available() > 0&&Serial3.readBytes(&tempbyte,1)==1)
    {
      GPSBuffer+=tempbyte;
      if(msl::ends_with(GPSBuffer,"\r\n"))
      {
        if(msl::starts_with(GPSBuffer,"$GPGGA")||msl::starts_with(GPSBuffer,"$GPRMC"))
        {
          boolean good = gps.update(GPSBuffer);
#ifdef SHOW_GPS_DAT
          Serial.print("parsed: ");
          Serial.println(good);
          Serial.print("GPS Lat: ");
          Serial.println(gps.lat);
          Serial.print("GPS Lng: ");
          Serial.println(gps.lng);
          Serial.print("GPS Alt: ");
          Serial.println(gps.alt);
          Serial.print("str: ");
          Serial.println(GPSBuffer.c_str());
#endif
        }

        GPSBuffer.clear();
      }
    }//end while

  }// end if


  /*
  //Take JPEG Image
   if(JPEGStatus[0] == 1)
   {
   if(!jpeg_taken_picture)
   {
   jpeg_taken_picture=jpeg_camera.takePicture();
   jpeg_total_size=jpeg_camera.frameLength();
   }
   else
   {
   long time=millis();
   
   while(jpeg_total_size>0&&millis()<time+200)
   {
   short buffer_size=jpeg_total_size;
   
   if(buffer_size>32)
   buffer_size=32;
   
   uint8_t* buffer=jpeg_camera.readPicture(buffer_size);
   jpeg_total_size-=buffer_size;
   std::string packet="dat";
   packet+=0x02;
   packet+=2+2+1+buffer_size;
   packet+=(char)(jpeg_seq&0xff);
   packet+=(char)((jpeg_seq<<8)&0xff);
   packet+=(char)(jpeg_total_size&0xff);
   packet+=(char)((jpeg_total_size<<8)&0xff);
   packet+=(char)buffer_size;
   
   for(short ii=0;ii<buffer_size;++ii)
   packet+=buffer[ii];
   
   packet+=calculate_crc(packet);
   Serial1.write((uint8_t*)packet.c_str(),packet.size());
   
   ++jpeg_seq;
   }
   
   if(jpeg_total_size==0)
   {
   jpeg_taken_picture=false;
   jpeg_seq=0;
   }
   }
   
  /*if(picture_stage==0)
   {
   cam.takePicture();
   ++picture_stage;
   }
   else if(picture_stage==1)
   {
   jpglen = cam.frameLength();
   JPEGtotalLength = jpglen;
   JPEGsequenceNum = 0;
   
   if(jpglen>0)
   {
   #ifdef  SHOW_JPEG_DAT
   //Serial.print("img1 of length\t");
   //Serial.println(jpglen);
   #endif
   ++picture_stage;
   }
   }
   else if(picture_stage==2)
   {
   if (jpglen > 0)
   {
   uint8_t bytesToRead=std::min((uint16_t)32, jpglen);
   uint8_t* buffer=cam.readPicture(bytesToRead);
   if(radioStatus[0] == 1)
   {
   Serial1.print("img1");
   Serial1.write( (uint8_t*) &JPEGsequenceNum, 2);
   Serial1.write( (uint8_t*) &JPEGtotalLength, 2);
   Serial1.write( bytesToRead);
   Serial1.write(buffer,bytesToRead);
   }
   jpglen -= bytesToRead;
   JPEGsequenceNum++;
   }
   else
   {
   ++picture_stage;
   }
   }
   else if(picture_stage==3)
   {
   cam.resumeVideo();
   picture_stage=0;
   #ifdef  SHOW_JPEG_DAT
   //Serial.println("Picture sent!");
   #endif
   }
   
   }
   else if(jpeg_taken_picture)
   {
   jpeg_camera.reset();
   jpeg_taken_picture=false;
   jpeg_seq=0;
   }
   */

}//end loop

// ================================================================
// ===           Camera Triggering Functions                    ===
// ================================================================

void shutterNow()
{
  bool seq[] = {
    1,0,1,1,0,1,0,0,1,0,1,1,1,0,0,0,1,1,1,1 
  };
  for (int j=0;j<3;j++) {
    high(2320,freq,LEDPin);
    wait(650);
    for (int i=0;i<sizeof(seq);i++){
      if (seq[i]==0){
        high(575,freq,LEDPin);
        wait(650);
      }
      else{
        high(1175,freq,LEDPin);
        wait(650);
      }
    }
    wait(10000);
  }
}

void wait(unsigned int time){
  unsigned long start = micros();
  while(micros()-start<=time){
  }
}

void high(unsigned int time, int freq, int pinLED){
  int pause = (1000/freq/2)-4;
  unsigned long start = micros();
  while(micros()-start<=time){
    digitalWrite(pinLED,HIGH);
    delayMicroseconds(pause);
    digitalWrite(pinLED,LOW);
    delayMicroseconds(pause);
  }
}

void CameraTriggered(void)
{
  pictureTakenFlag = true;
}

void HeartBeat(void)
{
  commArray[0] = 'd';
  commArray[1] = 'a';
  commArray[2] = 't';
  commArray[3] = 0x00;
  commArray[4] = 25;
  commArray[5] = SDStatus;
  commArray[6] = JPEGStatus[1];
  commArray[7] = NEXStatus[1];
  commArray[8] = ((char*)&gps.lat)[0];
  commArray[9] = ((char*)&gps.lat)[1];
  commArray[10] = ((char*)&gps.lat)[2];
  commArray[11] = ((char*)&gps.lat)[3];
  commArray[12] = ((char*)&gps.lng)[0];
  commArray[13] = ((char*)&gps.lng)[1];
  commArray[14] = ((char*)&gps.lng)[2];
  commArray[15] = ((char*)&gps.lng)[3];
  commArray[16] = ((char*)&gps.alt)[0];
  commArray[17] = ((char*)&gps.alt)[1];
  commArray[18] = ((char*)&gps.alt)[2];
  commArray[19] = ((char*)&gps.alt)[3];
  commArray[20] = gps.fix;
  commArray[21] = gps.sat_num;
  commArray[22] = ((char*)&gps.course)[0];
  commArray[23] = ((char*)&gps.course)[1];
  commArray[24] = ((char*)&gps.course)[2];
  commArray[25] = ((char*)&gps.course)[3];
  commArray[26] = ((char*)&gps.speed)[0];
  commArray[27] = ((char*)&gps.speed)[1];
  commArray[28] = ((char*)&gps.speed)[2];
  commArray[29] = ((char*)&gps.speed)[3];

  commArray[30] = 0;
  for(int ii = 0; ii < 30; ii++)
  {
    commArray[30] ^= commArray[ii];
  }

  Serial2.write( (uint8_t*)&commArray, 31);
  //  Serial1.print("dat");
  //
  //  Serial1.write((uint8_t)0x00);
  //  commNumBytesHB = 25;
  //  Serial1.write( (uint8_t*)&commNumBytesHB, 1);
  //  Serial1.write( (uint8_t*)&SDStatus,1);
  //  Serial1.write( (uint8_t*)&JPEGStatus[1],1);
  //  Serial1.write( (uint8_t*)&NEXStatus[1],1);
  //  Serial1.write( (uint8_t*)&gps.lat, 4);
  //  Serial1.write( (uint8_t*)&gps.lng, 4);  
  //  Serial1.write( (uint8_t*)&gps.alt, 4);
  //  Serial1.write( (uint8_t*)&gps.fix, 1);
  //  Serial1.write( (uint8_t*)&gps.sat_num, 1);
  //  Serial1.write( (uint8_t*)&gps.course, 4);
  //  Serial1.write( (uint8_t*)&gps.speed, 4);

  //  commCRC = 'd';
  //  commCRC ^= 'a';
  //  commCRC ^= 't';
  //  commCRC ^= 0x00;
  //  commCRC ^= commNumBytesHB;
  //  commCRC ^= SDStatus;
  //  commCRC ^= JPEGStatus[1];
  //  commCRC ^= NEXStatus[1];
  //  commCRC ^= ((uint8_t*)&gps.lat)[0];
  //  commCRC ^= ((uint8_t*)&gps.lat)[1];
  //  commCRC ^= ((uint8_t*)&gps.lat)[2];
  //  commCRC ^= ((uint8_t*)&gps.lat)[3];
  //  commCRC ^= ((uint8_t*)&gps.lng)[0];
  //  commCRC ^= ((uint8_t*)&gps.lng)[1];
  //  commCRC ^= ((uint8_t*)&gps.lng)[2];
  //  commCRC ^= ((uint8_t*)&gps.lng)[3];
  //  commCRC ^= ((uint8_t*)&gps.alt)[0];
  //  commCRC ^= ((uint8_t*)&gps.alt)[1];
  //  commCRC ^= ((uint8_t*)&gps.alt)[2];
  //  commCRC ^= ((uint8_t*)&gps.alt)[3];
  //  commCRC ^= gps.fix;
  //  commCRC ^= gps.sat_num;
  //  commCRC ^= ((uint8_t*)&gps.course)[0];
  //  commCRC ^= ((uint8_t*)&gps.course)[1];
  //  commCRC ^= ((uint8_t*)&gps.course)[2];
  //  commCRC ^= ((uint8_t*)&gps.course)[3];
  //  commCRC ^= ((uint8_t*)&gps.speed)[0];
  //  commCRC ^= ((uint8_t*)&gps.speed)[1];
  //  commCRC ^= ((uint8_t*)&gps.speed)[2];
  //  commCRC ^= ((uint8_t*)&gps.speed)[3];

  //  Serial1.write(commCRC);


#ifdef SHOW_HEARTBEAT
  //Serial.println("dat");
  //Serial.print("ID: ");
  //Serial.println(0);
  //Serial.print("Num Bytes: ");
  //Serial.println((int)commNumBytesHB);
  //Serial.print("SD Status: ");
  //Serial.println((int)SDStatus);
  //Serial.print("JPG Status: ");
  //Serial.println((int)JPEGStatus[1]);
  //Serial.print("NEx Status: ");
  //Serial.println((int)NEXStatus[1]);
  //Serial.print("UAV Lattitude: ");
  //Serial.println(gps.lat);
  //Serial.print("UAV Longitude: ");
  //Serial.println(gps.lng);
  //Serial.print("UAV Altitude: ");
  //Serial.println(gps.alt);
  //Serial.print("GPS Fix: ");
  //Serial.println(gps.fix);
  //Serial.print("Num Sat: ");
  //Serial.println(gps.sat_num);
  //Serial.print("UAV Course: ");
  //Serial.println(gps.course);
  //Serial.print("UAV Speed: ");
  //Serial.println(gps.speed);
  //Serial.print("Heartbeat CRC: ");
  //Serial.println((int)commArray[30]);

  //Serial.println('\n');
#endif
}
























































