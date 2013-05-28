#include "drone.hpp"

//File Utility Header
#include "msl/file_util.hpp"

//String Utility Header
#include "msl/string_util.hpp"

drone::drone(const unsigned char id,const std::string& serial_name,const unsigned int serial_baud):_id(id),_serial(serial_name,serial_baud),
	_serial_state(0),_serial_buffer(""),_stat_get_flags(0x00),_stat_set_flags(0x01),_img1_seq(0),_img1_data(""),_img2_angle(0.0),_img2_servo(0.0),
	_img2_size(0),_position(0.0,0.0,0.0)
{}

drone::drone(const drone& copy):_id(copy._id),_serial(copy._serial),_serial_state(copy._serial_state),_serial_buffer(copy._serial_buffer),
	_stat_get_flags(copy._stat_get_flags),_stat_set_flags(copy._stat_set_flags),_img1_seq(copy._img1_seq),_img1_data(copy._img1_data),
	_img2_angle(copy._img2_angle),_img2_servo(copy._img2_servo),_img2_size(copy._img2_size),_position(copy._position)
{}

drone& drone::operator=(const drone& copy)
{
	if(this!=&copy)
	{
		_id=copy._id;
		_serial=copy._serial;
		_serial_state=copy._serial_state;
		_serial_buffer=copy._serial_buffer;
		_stat_get_flags=copy._stat_get_flags;
		_stat_set_flags=copy._stat_set_flags;
		_img1_seq=copy._img1_seq;
		_img1_data=copy._img1_data;
		_img2_angle=copy._img2_angle;
		_img2_servo=copy._img2_servo;
		_img2_size=copy._img2_size;
		_position=copy._position;
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
				_serial_buffer.clear();

				if(byte=='s')
				{
					_serial_state=1;
				}
				else if(byte=='i')
				{
					_serial_state=4;
				}
			}
			else if(_serial_state==1)
			{
				if(byte=='t')
				{
					_serial_state=2;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==2)
			{
				if(byte=='a')
				{
					_serial_state=3;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==3)
			{
				if(byte=='t')
				{
					_serial_state=7;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==4)
			{
				if(byte=='m')
				{
					_serial_state=5;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==5)
			{
				if(byte=='g')
				{
					_serial_state=6;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==6)
			{
				if(byte=='1')
				{
					_serial_state=8;
				}
				else if(byte=='2')
				{
					_serial_state=9;
				}
				else
				{
					_serial_state=0;
				}
			}
			else if(_serial_state==7)
			{
				_serial_buffer+=byte;

				if(_serial_buffer.size()>=21)
				{
					stat_update(_serial_buffer);
					_serial_state=0;
				}
			}
			else if(_serial_state==8)
			{
				_serial_buffer+=byte;

				if(_serial_buffer.size()>=static_cast<unsigned int>(*(char*)(_serial_buffer.c_str()+4))+5)
				{
					img1_add_block(_serial_buffer);
					_serial_state=0;
				}
			}
			else if(_serial_state==9)
			{
				_serial_buffer+=byte;

				if(_serial_buffer.size()>=14)
				{
					img2_add_position(_serial_buffer);
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

char drone::stat_get() const
{
	return _stat_set_flags;
}

void drone::stat_set(const char flags)
{
	_stat_set_flags=flags;
	_serial<<"stat"<<_stat_set_flags;
}

location drone::position() const
{
	return _position;
}

char drone::stat_flags() const
{
	return _stat_get_flags;
}

float drone::img2_angle() const
{
	return _img2_angle;
}

float drone::img2_servo() const
{
	return _img2_servo;
}

short drone::img2_size() const
{
	return _img2_size;
}

std::map<short,location>& drone::img2_map()
{
	return _img2_positions;
}

const std::map<short,location>& drone::img2_map() const
{
	return _img2_positions;
}

void drone::stat_update(const std::string& packet)
{
	_stat_get_flags=packet[0];
	_img2_angle=*(float*)(packet.c_str()+1);
	_img2_servo=*(float*)(packet.c_str()+5);
	_position.lat=*(float*)(packet.c_str()+9);
	_position.lng=*(float*)(packet.c_str()+13);
	_position.alt=*(float*)(packet.c_str()+17);
}

void drone::img1_add_block(const std::string& packet)
{
	if(*(short*)(packet.c_str())==_img1_seq)
	{
		++_img1_seq;
		_img1_data+=std::string(packet.c_str()+5,static_cast<unsigned int>(*(char*)(_serial_buffer.c_str()+4)));

		if(_img1_data.size()>=static_cast<unsigned int>(*(short*)(packet.c_str()+2)))
		{
			std::string jpeg_footer="";
			jpeg_footer+=255;
			jpeg_footer+=217;
			_img1_seq=0;

			if(msl::ends_with(_img1_data,jpeg_footer))
			{
				std::string id_string=msl::to_string(static_cast<unsigned int>(_id));

				while(id_string.size()<3)
					id_string.insert(0,"0");

				std::string filename="web/drone/preview_"+id_string+".jpg";
				msl::string_to_file(_img1_data,filename);
				_img1_data.clear();
			}
		}
	}
	else
	{
		_img1_seq=0;
		_img1_data.clear();
	}
}

void drone::img2_add_position(const std::string& packet)
{
	_img2_size=*(short*)(packet.c_str());
	location temp(*(float*)(packet.c_str()+2),*(float*)(packet.c_str()+6),*(float*)(packet.c_str()+10));
	_img2_positions[_img2_size]=temp;
}