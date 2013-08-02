//Serial Sync Header
//	Created By:		Mike Moss
//	Modified On:	07/11/2013

//Begin Define Guards
#ifndef MSL_SERIALSYNC_H
#define MSL_SERIALSYNC_H

//Serial Header
#include "serial.hpp"

//String Header
#include <string>

//Vector Header
#include <vector>

//MSL Namespace
namespace msl
{
	//Serial Sync Class Declaration
	class serial_sync
	{
		public:
			//Constructor (Default)
			serial_sync(const std::string& port="",const unsigned int baud=57600);

			//Boolean Operator (Tests if Server is Good)
			operator bool() const;

			//Not Operator (For Boolean Operator)
			bool operator!() const;

			//Good Function (Tests if Serial Port is Good)
			bool good() const;

			//Setup Function (Sets Up Serial Port)
			void setup();

			//Update Function (Updates Data)
			void update();

			//Close Function (Closes Serial Port)
			void close();

			//Data Accessor (Non Const)
			short& operator[](unsigned int index);

			//Size Accessor
			unsigned int size() const;

		private:
			//Member Variables
			msl::serial _serial;
			int _serial_state;
			int _serial_receive_size;
			int _serial_receive_count;
			unsigned char _serial_receive_index;
			short _serial_receive_data;
			std::vector<short> _data;
	};
}

//End Define Guards
#endif