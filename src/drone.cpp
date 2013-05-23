#include "drone.hpp"

//File Utility Header
#include "msl/file_util.hpp"

//String Utility Header
#include "msl/string_util.hpp"

drone::drone(const unsigned char id,const std::string& serial_name,const unsigned int serial_baud):_id(id),_serial(serial_name,serial_baud),
	_serial_state(0),_serial_buffer(""),_flags(0x00),_img1_seq(0),_img1_data(""),_img2_angle(0),_img2_servo(0),_location(0,0,0)
{}

drone::drone(const drone& copy):_id(copy._id),_serial(copy._serial),_serial_state(copy._serial_state),_serial_buffer(copy._serial_buffer),_flags(copy._flags),
	_img1_seq(copy._img1_seq),_img1_data(copy._img1_data),_img2_angle(copy._img2_angle),_img2_servo(copy._img2_servo),_location(copy._location)
{}

drone& drone::operator=(const drone& copy)
{
	if(this!=&copy)
	{
		_id=copy._id;
		_serial=copy._serial;
		_serial_state=copy._serial_state;
		_serial_buffer=copy._serial_buffer;
		_flags=copy._flags;
		_img1_seq=copy._img1_seq;
		_img1_data=copy._img1_data;
		_img2_angle=copy._img2_angle;
		_img2_servo=copy._img2_servo;
		_location=copy._location;
	}

	return *this;
}

void drone::connect()
{
	_serial.connect();
}

void drone::close()
{
	_serial.close();
}

void drone::update()
{
	if(good())
	{
		char byte='\n';

		while(_serial.available()>0&&_serial.read(&byte,1)==1)
		{
			if(_serial_state==0)
			{
				if(byte=='s')
					_serial_state=1;
				else if(byte=='c')
					_serial_state=4;
			}
			else if(_serial_state==1)
			{
				if(byte=='t')
					_serial_state=2;
				else
					_serial_state=0;
			}
			else if(_serial_state==2)
			{
				if(byte=='a')
					_serial_state=3;
				else
					_serial_state=0;
			}
			else if(_serial_state==3)
			{
				if(byte=='t')
					_serial_state=7;
			}
			else if(_serial_state==4)
			{
				if(byte=='a')
					_serial_state=5;
				else
					_serial_state=0;
			}
			else if(_serial_state==5)
			{
				if(byte=='m')
					_serial_state=6;
				else
					_serial_state=0;
			}
			else if(_serial_state==6)
			{
				if(byte=='1')
					_serial_state=8;
				else if(byte=='2')
					_serial_state=9;
				else
					_serial_state=0;
			}
			else if(_serial_state==7)
			{
				if(_serial_buffer.size()<14)
				{
					_serial_buffer+=byte;
				}
				else
				{
					stat_set(_serial_buffer);
					_serial_state=0;
				}
			}
			else if(_serial_state==8)
			{
				if(_serial_buffer.size()<5||(_serial_buffer.size()>=5&&_serial_buffer.size()-5<*(char*)(_serial_buffer.c_str()+5)))
				{
					_serial_buffer+=byte;
				}
				else
				{
					img1_add_block(_serial_buffer);
					_serial_state=0;
				}
			}
			else if(_serial_state==9)
			{
				if(_serial_buffer.size()<12)
				{
					_serial_buffer+=byte;
				}
				else
				{
					img2_add_location(_serial_buffer);
					_serial_state=0;
				}
			}
		}
	}
}

unsigned char drone::id() const
{
	return _id;
}

bool drone::good() const
{
	return _serial.good();
}

bool drone::stat_set(const std::string& packet)
{
	if(packet.size()==17)
	{
		_flags=packet[0];
		_img2_angle=*(short*)(packet.c_str()+1);
		_img2_servo=*(short*)(packet.c_str()+3);
		_location.lat=*(float*)(packet.c_str()+5);
		_location.lng=*(float*)(packet.c_str()+9);
		_location.alt=*(float*)(packet.c_str()+13);
		return true;
	}

	return false;
}

bool drone::img1_add_block(const std::string& packet)
{
	if(packet.size()>=5)
	{
		if(*(short*)(packet.c_str())==_img1_seq)
		{
			_img1_data+=(packet.c_str()+5);

			if(_img1_data.size()==*(short*)(packet.c_str()+2))
			{
				std::string jpeg_footer="";
				jpeg_footer+=255;
				jpeg_footer+=217;

				if(msl::ends_with(_img1_data,jpeg_footer))
				{
					std::string prefix="img_";
					std::string midfix="";
					std::string postfix=".jpg";

					for(int place=10;place<10000;place*=10)
						midfix+="0";

					std::string filename=prefix+midfix+msl::to_string(_img1_seq)+postfix;
					msl::string_to_file(_img1_data,filename);

					++_img1_seq;
					_img1_data.clear();
				}
			}
		}
		else
		{
			_img1_seq=*(short*)(packet.c_str());
			_img1_data.clear();
		}
	}
}

bool drone::img2_add_location(const std::string& packet)
{
	if(packet.size()==12)
	{
		location temp(*(float*)(packet.c_str()),*(float*)(packet.c_str()+4),*(float*)(packet.c_str()+8));
		_img2_locations.push_back(temp);
	}
}