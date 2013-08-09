//GPS Node Source
//	Created By:		Mike Moss
//	Modified On:	05/29/2013

//Definitions for "gps_node.h"
#include "gps_node.h"

//GPS Utility Header
#include "gps_util.h"

//String Stream Header
#include <sstream>

//String Utility Header
#include "string_util.h"

//Update Function (Pass in GPS Sentences)
bool gps_node::update(const std::string& buffer)
{
  //Check for Valid String
  if(msl::starts_with(buffer,"$GP")&&msl::ends_with(buffer,"\r\n")&&gps_checksum(buffer))
  {
    //Create a Parsable String
    std::string parse_string=buffer;

    //Look for Empty Entries and Change ','s to ' 's
    for(unsigned int ii=0;ii<parse_string.size();++ii)
    {
      if(parse_string[ii]==',')
      {
        parse_string[ii]=' ';

        if(ii+1<parse_string.size()&&parse_string[ii+1]==',')
        {
          parse_string.insert(ii+1,"nan");
        }
      }
    }

    //String Streams Used for Parsing (One for Data one for Empty Entry Checking)
    std::istringstream istr(parse_string,std::ios_base::in);
    std::istringstream istr_nan(parse_string,std::ios_base::in);

    //Get Header (From Both Streams)
    std::string header;
    istr>>header;
    istr_nan>>header;

    //Global Positioning System Fix Data
    if(header=="$GPGGA")
    {
      //Parsing Variables
      char temp_char;
      std::string temp_str;

      //Parse Time
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_str;
        time=gps_hhmmss_ss_to_seconds(temp_str);
        if(time!=time)
          time=0.0;
        temp_str="";
      }
      else
        return false;

      //Parse Lattitude
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        //Get Lattitude
        temp_str="";
        istr>>temp_str;
        lat=gps_degrees_minutes_to_degrees(temp_str);
        if(lat!=lat)
          lat=0.0;
        temp_str="";

        //Get Direction
        if(istr_nan>>temp_str&&temp_str!="NAN")
          if(istr>>temp_char&&temp_char=='S'||temp_char=='s')
            lat*=-1.0;

        //Clear Temp String
        temp_str="";
      }
      else
        return false;

      //Parse Longitude
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        //Get Lattitude
        temp_str="";
        istr>>temp_str;
        lng=gps_degrees_minutes_to_degrees(temp_str);
        if(lng!=lng)
          lng=0.0;
        temp_str="";

        //Get Direction
        if(istr_nan>>temp_str&&temp_str!="NAN")
          if(istr>>temp_char&&temp_char=='W'||temp_char=='w')
            lng*=-1.0;

        //Clear Temp String
        temp_str="";
      }
      else
        return false;

      //Parse Fix
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>fix;
      }
      else
        return false;

      //Parse Number of Satellites
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>sat_num;
      }
      else
        return false;

      //Parse Horizontal Dilution of Precision (Ignored)
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        istr>>temp_str;
        temp_str="";
      }
      else
        return false;

      //Parse Altitude
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_str;
        alt=msl::to_double(temp_str);
        if(alt!=alt)
          alt=0.0;
        temp_str="";
      }
      else
        return false;

      //Good String
      return true;
    }

    //Recommended Minimum Specific GPS/Transit Data
    else if(header=="$GPRMC")
    {
      //Parsing Variables
      char temp_char;
      std::string temp_str;

      //NAN Parser
      std::istringstream istr_nan(istr.str(),std::ios_base::in);

      //Parse Time
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_str;
        time=gps_hhmmss_ss_to_seconds(temp_str);
        if(time!=time)
          time=0.0;
        temp_str="";
      }
      else
        return false;

      //Parse Data Status (Ignored)
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_char;
      }
      else
        return false;

      //Parse Lattitude
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        //Get Lattitude
        temp_str="";
        istr>>temp_str;
        lat=gps_degrees_minutes_to_degrees(temp_str);
        if(lat!=lat)
          lat=0.0;
        temp_str="";

        //Get Direction
        if(istr_nan>>temp_str&&temp_str!="NAN")
          if(istr>>temp_char&&temp_char=='S'||temp_char=='s')
            lat*=-1.0;

        //Clear Temp String
        temp_str="";
      }
      else
        return false;

      //Parse Longitude
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        //Get Lattitude
        temp_str="";
        istr>>temp_str;
        lng=gps_degrees_minutes_to_degrees(temp_str);
        if(lng!=lng)
          lng=0.0;
        temp_str="";

        //Get Direction
        if(istr_nan>>temp_str&&temp_str!="NAN")
          if(istr>>temp_char&&temp_char=='W'||temp_char=='w')
            lng*=-1.0;

        //Clear Temp String
        temp_str="";
      }
      else
        return false;

      //Parse Speed
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_str;
        speed=msl::to_double(temp_str);
        if(speed!=speed)
          speed=0.0;
        temp_str="";
      }
      else
        return false;

      //Parse Course
      if(istr_nan>>temp_str&&temp_str!="NAN")
      {
        temp_str="";
        istr>>temp_str;
        course=msl::to_double(temp_str);
        if(course!=course)
          course=0.0;
        temp_str="";
      }
      else
        return false;

      //Good String
      return true;
    }
  }

  //Bad String
  return false;
}

