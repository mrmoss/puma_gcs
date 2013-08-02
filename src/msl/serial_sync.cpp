//Serial Sync Source
//	Created By:		Mike Moss
//	Modified On:	07/11/2013

//Definitions for "serial_sync.hpp"
#include "serial_sync.hpp"

//Exception Header
#include <stdexcept>

//Constructor (Default)
msl::serial_sync::serial_sync(const std::string& port,const unsigned int baud):_serial(port,baud),
	_serial_state(0),_serial_receive_size(0),_serial_receive_count(0),_serial_receive_index(0),_serial_receive_data(0)
{}

//Boolean Operator (Tests if Server is Good)
msl::serial_sync::operator bool() const
{
	return good();
}

//Not Operator (For Boolean Operator)
bool msl::serial_sync::operator!() const
{
	return !good();
}

//Good Function (Tests if Serial Port is Good)
bool msl::serial_sync::good() const
{
	return _serial.good();
}

//Setup Function (Sets Up Serial Port)
void msl::serial_sync::setup()
{
	_serial.connect();
}

//Update Function (Updates Data)
void msl::serial_sync::update()
{
	if(good())
	{
		unsigned char temp;

		while(_serial.available()>0&&_serial.read(&temp,1)==1)
		{
			//Parse Header (uaf)
			if(_serial_state==0&&temp=='u')
			{
				_serial_state=1;
			}
			else if(_serial_state==1&&temp=='a')
			{
				_serial_state=2;
			}
			else if(_serial_state==2&&temp=='f')
			{
				_serial_state=3;
			}

			//Parse Number of Values
			else if(_serial_state==3&&temp>0)
			{
				_serial_receive_size=static_cast<int>(temp);
				_serial_state=4;
			}

			//Parse Value Index
			else if(_serial_state==4)
			{
				//Get Index
				_serial_receive_index=temp;

				//Make Unsigned Int Version for Vector Resize
				unsigned int index=static_cast<unsigned int>(_serial_receive_index);

				//Resize If Necessary
				if(index>=_data.size())
					_data.resize(index+1);

				//Next State
				_serial_state=5;
			}
			else if(_serial_state==5)
			{
				//Read First Byte of Data
				reinterpret_cast<unsigned char*>(&_serial_receive_data)[0]=temp;
				_serial_state=6;
			}
			else if(_serial_state==6)
			{
				//Read First Byte of Data
				reinterpret_cast<unsigned char*>(&_serial_receive_data)[1]=temp;

				//Write Data
				_data[_serial_receive_index]=_serial_receive_data;

				//Increment Data Received Counter
				++_serial_receive_count;

				//Next State
				_serial_state=4;
			}
			else
			{
				_serial_state=0;
			}
		}
	}
}

//Close Function (Closes Serial Port)
void msl::serial_sync::close()
{
	_serial.close();
}

//Data Accessor (Non Const)
short& msl::serial_sync::operator[](unsigned int index)
{
	//Max 255 Variables
	if(index>255)
		throw std::runtime_error("msl::serial_sync::operator[] - Invalid index!");

	//Resize If Necessary
	if(index>=_data.size())
		_data.resize(index+1);

	//Return Data
	return _data[index];
}

//Size Accessor
unsigned int msl::serial_sync::size() const
{
	return _data.size();
}