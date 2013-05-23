#ifndef PUMA_GCS_DRONE_H
#define PUMA_GCS_DRONE_H

//Location Header
#include "location.hpp"

//Serial Header
#include "msl/serial.hpp"

//String Header
#include <string>

//Vector Header
#include <vector>

//Puma Class
class drone
{
	public:
		//Constructor (Default)
		drone(const unsigned char id=0,const std::string& serial_name="",const unsigned int serial_baud=0);

		//Copy Constructor
		drone(const drone& copy);

		//Copy Assignment Operator
		drone& operator=(const drone& copy);

		//Connect Function
		void connect();

		//Close Function
		void close();

		//Update Function
		void update();

		//ID Accessor
		unsigned char id() const;

		//Serial Port Good Accessor
		bool good() const;

	private:
		//Member Functions
		bool stat_set(const std::string& packet);
		bool img1_add_block(const std::string& packet);
		bool img2_add_location(const std::string& packet);

		//Member Variables
		unsigned char _id;
		msl::serial _serial;
		int _serial_state;
		std::string _serial_buffer;
		char _flags;
		short _img1_seq;
		std::string _img1_data;
		short _img2_angle;
		short _img2_servo;
		location _location;
		std::vector<location> _img2_locations;
};

#endif