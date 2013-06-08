//UAV Header
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//Begin Define Guards
#ifndef GCS_UAV_H
#define GCS_UAV_H

//Location Header
#include "location.hpp"

//Map Header
#include <map>

//Serial Header
#include "msl/serial.hpp"

//UAV Class Declaration
class uav
{
	public:
		//Constructor (Default)
		uav(const unsigned char ID=0,const std::string serial_port="",const unsigned int serial_baud=0);

		//Update Function (Checks for bytes on radio and updates members)
		void update();

		//Connect Function (Connect to serial and returns result).
		bool connect();

		//Close Function (Closes and releases serial port)
		void close();

		//Change Hardware Function (Changes the state of hardware over the radio, 1 is radio,
		//	2 is the JPG camera and 3 is the NEX camera)
		void change_hw(const unsigned char id,const bool state);

		//JSON Function (Builds json object of members)
		std::string json() const;

		//ID Accessor
		unsigned char id() const;

		//Serial Port Name Accessor
		std::string serial_name() const;

		private:
			//Private Member Variables
			unsigned char _id;
			std::string _serial_port;
			unsigned int _serial_baud;
			msl::serial _serial;
			int _packet_state;
			std::string _packet_buffer;
			unsigned char _sd;
			unsigned char _jpg;
			unsigned char _nex;
			location _pos;
			unsigned char _fix;
			unsigned char _sats;
			float _course;
			float _speed;
			std::string _jpg_data;
			short _jpg_seq;
			std::map<short,location> _nex_locations;

};

//End Define Guards
#endif