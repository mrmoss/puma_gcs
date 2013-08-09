//GPS Utilities Source
//	Created By:		Mike Moss
//	Modified On:	05/29/2013

//Definitions for "gps_util.h"
#include "gps_util.h"

//String Utility Header
#include "string_util.h"

//GPS Checksum Function (Give complete sentence (with $ at beginning \r\n at end)
bool gps_checksum(const std::string& gps_sentence)
{
  //Check for Start and Ending Characters
  if(gps_sentence.size()>4&&gps_sentence[0]=='$'&&gps_sentence[gps_sentence.size()-5]=='*')
  {
    //Checksum Variable
    char checksum=0;

    //Calculate the Checksum
    for(unsigned int ii=1;ii<gps_sentence.size()-5;++ii)
      checksum^=gps_sentence[ii];

    //Get the Test Value
    std::string test_string=gps_sentence.substr(gps_sentence.size()-4,2);
    unsigned int test=0;

    //Convert Test Value to a Number
    if(test_string.size()==2)
    {
      if(test_string[0]>='a'&&test_string[0]<='f')
        test+=(test_string[0]-'a'+10)*16;
      else if(test_string[0]>='A'&&test_string[0]<='F')
        test+=(test_string[0]-'A'+10)*16;
      else if(test_string[0]>='0'&&test_string[0]<='9')
        test+=(test_string[0]-'0')*16;

      if(test_string[1]>='a'&&test_string[1]<='f')
        test+=(test_string[1]-'a'+10);
      else if(test_string[1]>='A'&&test_string[1]<='F')
        test+=(test_string[1]-'A'+10);
      else if(test_string[1]>='0'&&test_string[1]<='9')
        test+=(test_string[1]-'0');
    }

    //Return Result
    return (checksum==test);
  }

  //Bad Sentence
  return false;
}

//Convert GPS Time to Seconds
float gps_hhmmss_ss_to_seconds(const std::string& buffer)
{
  //Check for Valid String
  if(buffer.size()>6)
  {
    //Get Time Components
    std::string hours=buffer.substr(0,2);
    std::string mins=buffer.substr(2,2);
    std::string secs=buffer.substr(4,buffer.size()-4);

    //Return Time
    return msl::to_double(hours)*3600+msl::to_double(mins)*60+msl::to_double(secs);
  }

  //Bad Buffer String
  return 0.0;
}

//Convert GPS Degrees Minutes to Degrees
float gps_degrees_minutes_to_degrees(const std::string& buffer)
{
  //Variables
  float return_value=0.0;
  std::string degrees;

  //Get Whole Number Part
  for(unsigned int ii=0;ii<buffer.size();++ii)
    if(buffer[ii]!='.')
      degrees+=buffer[ii];
    else
      break;

  //Get Degrees
  degrees.erase(degrees.size()-2,2);
  return_value=msl::to_int(degrees);

  //Get Minutes
  std::string minutes_str=buffer;
  minutes_str.erase(0,degrees.size());
  return_value+=msl::to_double(minutes_str)/60.0;

  //Return Value
  return return_value;
}
