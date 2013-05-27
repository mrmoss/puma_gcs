#ifndef PUMA_GCS_DRONE_H
#define PUMA_GCS_DRONE_H

//Location Header
#include "location.hpp"

//Map Header
#include <map>

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

		//Stat Accessor
		char stat_get() const;

		//Stat Mutator
		void stat_set(const char flags);

		//Position Accessor
		location position() const;

		char stat_flags() const;
		float img2_angle() const;
		float img2_servo() const;
		short img2_size() const;
		std::map<short,location>& img2_map();
		const std::map<short,location>& img2_map() const;

	//private:
		//Member Functions
		void stat_update(const std::string& packet);
		void img1_add_block(const std::string& packet);
		void img2_add_position(const std::string& packet);

		//Member Variables
		unsigned char _id;
		msl::serial _serial;
		int _serial_state;
		std::string _serial_buffer;
		char _stat_get_flags;
		char _stat_set_flags;
		short _img1_seq;
		std::string _img1_data;
		float _img2_angle;
		float _img2_servo;
		short _img2_size;
		location _position;
		std::map<short,location> _img2_positions;
};

#endif