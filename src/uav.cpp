//UAV Source
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//Definitions for "uav.hpp"
#include "uav.hpp"

//CRC Header
#include "crc.hpp"

//File Stream Header
#include <fstream>

//File Utility Header
#include "msl/file_util.hpp"

//JSON Header
#include "msl/json.hpp"

//Time Utility Header
#include "msl/time_util.hpp"

//UAV Constructor (Default)
uav::uav(const unsigned char id,const std::string serial_port,const unsigned int serial_baud):_id(id),_serial_port(serial_port),_serial_baud(serial_baud),
	_packet_state(0),_packet_buffer(""),_sd(0),_hw_timer(msl::millis()),_radio_desired_state(false),_jpg_desired_state(false),_nex_desired_state(false),_jpg(0),
	_nex(0),_pos(0.0,0.0,0.0),_fix(0),_sats(0),_course(0.0),_speed(0.0),_heartbeat_timer(msl::millis())
{
	//Create Log Filename (template is XXX.log)
	_log_name=msl::to_string(static_cast<unsigned int>(_id));
	while(_log_name.size()<3)
		_log_name.insert(0,"0");
	_log_name.insert(0,"web/");
	_log_name+=".log";

	//Log
	log("Created log file.\n",false);
}

//Update Function (Checks for bytes on radio and updates members)
void uav::update()
{
	//Temporary Byte
	char read_byte;

	//While There's Bytes to Read
	while(_serial.available()>0&&_serial.read(&read_byte,1)==1)
	{
		//Get 'd' of Header
		if(_packet_state==0&&read_byte=='d')
		{
			_packet_buffer+=read_byte;
			_packet_state=1;
		}

		//Get 'a' of Header
		else if(_packet_state==1&&read_byte=='a')
		{
			_packet_buffer+=read_byte;
			_packet_state=2;
		}

		//Get 't' of Header
		else if(_packet_state==2&&read_byte=='t')
		{
			_packet_buffer+=read_byte;
			_packet_state=3;
		}

		//Get ID and Payload Size
		else if(_packet_state==3||_packet_state==4)
		{
			_packet_buffer+=read_byte;
			++_packet_state;
		}

		//Get Payload
		else if(_packet_state==5)
		{
			_packet_buffer+=read_byte;

			if(_packet_buffer.size()==static_cast<unsigned int>(_packet_buffer[4])+5)
				_packet_state=6;
		}

		//Get CRC
		else if(_packet_state==6)
		{
			//If CRCs Match
			if(read_byte==calculate_crc(_packet_buffer))
			{
				//Heartbeat Packet
				if(_packet_buffer[3]==0)
				{
					//Extract Heartbeat Information
					_sd=_packet_buffer[5];
					_jpg=_packet_buffer[6];
					_nex=_packet_buffer[7];
					_pos.lat=*(float*)&_packet_buffer[8];
					_pos.lng=*(float*)&_packet_buffer[12];
					_pos.alt=*(float*)&_packet_buffer[16];
					_fix=_packet_buffer[20];
					_sats=_packet_buffer[21];
					_course=*(float*)&_packet_buffer[22];
					_speed=*(float*)&_packet_buffer[26];
					_heartbeat_timer=msl::millis();

					//Log
					log("Heartbeat @ "+msl::to_string(msl::millis())+"ms\n",true);
				}

				//JPG Block Packet
				else if(_packet_buffer[3]==2)
				{
					//If Sequences Match
					if(*(short*)&_packet_buffer[5]==_jpg_seq)
					{
						//Increase Sequence
						++_jpg_seq;

						//Add JPG Data
						_jpg_data+=std::string(_packet_buffer[10],_packet_buffer[9]);

						//If We Have All the JPG Data Blocks
						if(static_cast<short>(_jpg_data.size())==*(short*)&_packet_buffer[7])
						{
							//Create Filename (template is jpg_XXX.jpg)
							std::string filename=msl::to_string(static_cast<unsigned int>(_id));
							while(filename.size()<3)
								filename.insert(0,"0");

							//Save File
							msl::string_to_file("jpg_"+filename+".jpg",_jpg_data);

							//Reset JPG Data Variables
							_jpg_data="";
							_jpg_seq=0;
						}

						//Log
						log("Recieved JPG block @ "+msl::to_string(msl::millis())+"ms\n",true);
					}

					//If Sequences Do Not Match Reset JPG Data Variables
					else
					{
						//Reset JPG Image
						_jpg_data="";
						_jpg_seq=0;

						//Log
						log("Invalid JPG block detected @ "+msl::to_string(msl::millis())+"ms\n",true);
					}
				}

				//NEX Location Packet
				else if(_packet_buffer[3]==3)
				{
					//Extract Location Data
					short nex_seq=*(short*)&_packet_buffer[5];
					float nex_lat=*(float*)&_packet_buffer[7];
					float nex_lng=*(float*)&_packet_buffer[11];
					float nex_alt=*(float*)&_packet_buffer[15];

					//Add Location to Location Vector
					_nex_locations[nex_seq]=location(nex_lat,nex_lng,nex_alt);

					//Log
					log("Received new NEX location @ "+msl::to_string(msl::millis())+"ms\n",true);
				}

				//Bad Packet ID(took out debug messages)
				else
				{
					//std::cout<<"unknown packet id"<<std::endl;

					//Log
					log("Received bad packet id ("+msl::to_string(static_cast<unsigned int>(_packet_buffer[3]))+") @ "+msl::to_string(msl::millis())+"ms\n",true);
				}
			}

			//If CRCs Do Not Match (took out debug messages)
			else
			{
				//std::cout<<"bad packet"<<std::endl;

				//Log
				log("Invalid CRC packet detected @ "+msl::to_string(msl::millis())+"ms\n",true);
			}

			//Either Way Reset Parse State
			_packet_state=0;
			_packet_buffer="";
		}

		//Reset Parse State
		else
		{
			_packet_state=0;
			_packet_buffer="";
		}
	}

	//Update Hardware States
	if(msl::millis()-_hw_timer>=1000)
	{
		//Update Radio
		change_hw(0x01,_radio_desired_state);

		//Update JPG Camera
		if(static_cast<bool>(_jpg)!=_jpg_desired_state)
			change_hw(0x02,_jpg_desired_state);

		//Update NEX Camera
		if(static_cast<bool>(_nex)!=_nex_desired_state)
			change_hw(0x03,_nex_desired_state);

		//Update Timer
		_hw_timer=msl::millis();
	}
}

//Connect Function (Connect to serial and returns result).
bool uav::connect()
{
	//Set Serial Port
	_serial=msl::serial(_serial_port,_serial_baud);

	//Connect
	_serial.connect();

	//Log
	if(_serial.good())
		log("Connected to serial port.\n",true);
	else
		log("Could not connect to serial port.\n",true);

	//Return Result
	return _serial.good();
}

//Close Function (Closes and releases serial port)
void uav::close()
{
	//Disconnect Serial Port
	if(_serial.good())
		_serial.close();

	//Log
	log("Disconnected from serial port.\n",true);
}

//Set Hardware Function (Changes the desired state of hardware, 1 is radio,
//	2 is the JPG camera and 3 is the NEX camera)
void uav::set_hw(const unsigned char id,const bool state)
{
	//std::cout<<"Setting desired hardware setting of id "<<std::hex<<(unsigned short)id<<" to "<<std::dec<<state<<std::endl;

	//Radio
	if(id==0x01)
		_radio_desired_state=state;

	//JPG Camera
	else if(id==0x02)
		_jpg_desired_state=state;

	//NEX Camera
	else if(id==0x03)
		_nex_desired_state=state;
}

//Change Hardware Function (Changes the state of hardware over the radio, 1 is radio,
//	2 is the JPG camera and 3 is the NEX camera)
void uav::change_hw(const unsigned char id,const bool state)
{
	//Check Serial Port
	if(_serial.good())
	{
		//Log
		log("Sending hardware change ("+msl::to_string(static_cast<unsigned int>(id))+") to ",true);

		if(state)
			log("true",true);
		else
			log("false",true);

		log(" @ "+msl::to_string(msl::millis())+"ms\n",true);

		//Add Header
		std::string packet="dat";

		//Add ID
		packet+=id;

		//Add Paylod Size
		packet+=static_cast<char>(0x01);

		//Add State
		if(!state)
			packet+=static_cast<char>(0x00);
		else
			packet+=static_cast<char>(0x01);

		//Add CRC
		packet+=calculate_crc(packet);

		//Send Packet
		_serial<<packet;
	}
}

//Log Function (Writes to Log)
void uav::log(const std::string& entry,const bool append) const
{
	std::ofstream ostr;

	if(append)
		ostr.open(_log_name.c_str(),std::fstream::app);
	else
		ostr.open(_log_name.c_str());

	if(ostr)
	{
		ostr<<entry;
		ostr.close();
	}
}

//JSON Function (Builds json object of members)
std::string uav::json() const
{
	//Create Response Object
	msl::json response;

	//Create Response Status Object
	msl::json status;
	status.set("id",static_cast<unsigned int>(_id));
	status.set("serial_port",_serial_port);
	status.set("serial_baud",_serial_baud);
	status.set("serial_status",static_cast<unsigned int>(_serial.good()));
	status.set("sd",static_cast<unsigned int>(_sd));
	status.set("radio",static_cast<unsigned int>(_radio_desired_state));
	status.set("jpg",static_cast<unsigned int>(_jpg_desired_state));
	status.set("nex",static_cast<unsigned int>(_nex_desired_state));
	status.set("lat",_pos.lat);
	status.set("lng",_pos.lng);
	status.set("alt",_pos.alt);
	status.set("fix",static_cast<unsigned int>(_fix));
	status.set("course",_course);
	status.set("speed",_speed);
	status.set("heartbeat",msl::millis()-_heartbeat_timer);
	response.set("status",status.str());

	//Create Response JPG Object
	msl::json jpg_camera;
	std::string jpg_filename=msl::to_string(static_cast<unsigned int>(_id));

	while(jpg_filename.size()<3)
		jpg_filename.insert(0,"0");

	jpg_camera.set("src","jpg_"+jpg_filename+".jpg");
	response.set("jpg",jpg_camera.str());

	//Create Response NEX Object
	msl::json nex_camera;
	nex_camera.set("size",_nex_locations.size());

	unsigned int nex_count=0;

	for(std::map<short,location>::const_iterator iter=_nex_locations.begin();iter!=_nex_locations.end();++iter)
	{
		msl::json loc_json;
		loc_json.set("seq",iter->first);
		loc_json.set("lat",iter->second.lat);
		loc_json.set("lng",iter->second.lng);
		loc_json.set("alt",iter->second.alt);
		nex_camera.set(msl::to_string(nex_count),loc_json.str());
		++nex_count;
	}

	response.set("nex",nex_camera.str());

	//Return Response Object String
	return response.str();
}

//ID Accessor
unsigned char uav::id() const
{
	return _id;
}

//Serial Port Name Accessor
std::string uav::serial_name() const
{
	return _serial_port;
}