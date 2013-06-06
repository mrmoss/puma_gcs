#include <cstdlib>
#include "msl/file_util.hpp"
#include <iostream>
#include "msl/json.hpp"
#include "msl/serial.hpp"
#include "msl/socket.hpp"
#include "msl/socket_util.hpp"
#include <string>
#include "msl/string_util.hpp"
#include <vector>

char calculate_crc(const std::string& packet)
{
	char calc=0x00;

	for(unsigned int ii=0;ii<packet.size();++ii)
		calc^=packet[ii];

	return calc;
}

class uav
{
	public:
		uav(const char ID,const std::string serial_port,const unsigned int serial_baud):id(ID),_serial_port(serial_port),_serial_baud(serial_baud),
			_packet_state(0),_packet_buffer(""),sd(1),jpg(1),nex(0),lat(64.85707646953384),lng(-147.82556247711182),alt(200.12),fix(2),sats(2),course(30.4),speed(15.3)
		{}

		std::string change_hw(const char id,const bool state)
		{
			std::string packet="dat";
			packet+=id;
			packet+=static_cast<char>(0x01);

			if(!state)
				packet+=static_cast<char>(0x00);
			else
				packet+=static_cast<char>(0x01);

			packet+=calculate_crc(packet);
			return packet;
		}

		bool setup_serial()
		{
			_serial=msl::serial(_serial_port,_serial_baud);
			_serial.connect();
			return _serial.good();
		}

		void update_serial()
		{
			char read_byte;

			while(_serial.available()>0&&_serial.read(&read_byte,1)==1)
			{
				//Get Data Packet
				if(_packet_state==0&&read_byte=='d')
				{
					_packet_buffer+=read_byte;
					_packet_state=1;
				}
				else if(_packet_state==1&&read_byte=='a')
				{
					_packet_buffer+=read_byte;
					_packet_state=2;
				}
				else if(_packet_state==2&&read_byte=='t')
				{
					_packet_buffer+=read_byte;
					_packet_state=3;
				}
				else if(_packet_state==3||_packet_state==4)
				{
					_packet_buffer+=read_byte;
					++_packet_state;
				}
				else if(_packet_state==5)
				{
					_packet_buffer+=read_byte;

					if(_packet_buffer.size()==static_cast<unsigned int>(_packet_buffer[4])+5)
						_packet_state=10;
				}

				//Get Response Packet
				else if(_packet_state==0&&read_byte=='r')
				{
					_packet_buffer+=read_byte;
					_packet_state=6;
				}
				else if(_packet_state==6&&read_byte=='e')
				{
					_packet_buffer+=read_byte;
					_packet_state=7;
				}
				else if(_packet_state==7&&read_byte=='s')
				{
					_packet_buffer+=read_byte;
					_packet_state=8;
				}
				else if(_packet_state==8)
				{
					_packet_buffer+=read_byte;
					_packet_state=9;
				}
				else if(_packet_state==9)
				{
					_packet_buffer+=read_byte;

					if(_packet_buffer.size()==6)
						_packet_state=10;
				}

				//CRC Check
				else if(_packet_state==10)
				{
					if(read_byte==calculate_crc(_packet_buffer))
					{
						if(msl::starts_with(_packet_buffer,"dat"))
						{
							if(_packet_buffer[3]==0)
							{
								sd=_packet_buffer[5];
								jpg=_packet_buffer[6];
								nex=_packet_buffer[7];
								lat=*(float*)&_packet_buffer[8];
								lng=*(float*)&_packet_buffer[12];
								alt=*(float*)&_packet_buffer[16];
								fix=_packet_buffer[20];
								sats=_packet_buffer[21];
								course=*(float*)&_packet_buffer[22];
								speed=*(float*)&_packet_buffer[26];
							}

							else if(_packet_buffer[3]==2)
							{
								if(*(short*)&_packet_buffer[5]==_jpg_seq)
								{
									++_jpg_seq;
									_jpg_data+=std::string(_packet_buffer[10],_packet_buffer[9]);

									if(static_cast<short>(_jpg_data.size())==*(short*)&_packet_buffer[7])
									{
										std::string filename=msl::to_string(id);
										while(filename.size()<3)
											filename.insert(0,"0");

										msl::string_to_file("jpg_"+filename+".jpg",_jpg_data);
										_jpg_data="";
										_jpg_seq=0;
									}
								}
								else
								{
									_jpg_data="";
									_jpg_seq=0;
								}
							}

							else
							{
								std::cout<<"unknown packet id"<<std::endl;
							}
						}
						else
						{
							std::cout<<"unknown packet header"<<std::endl;
						}
					}
					else
					{
						std::cout<<"bad packet"<<std::endl;
					}

					_packet_state=0;
					_packet_buffer="";
				}

				//Garbage
				else
				{
					_packet_state=0;
					_packet_buffer="";
				}
			}
		}

		std::string info() const
		{
			msl::json response;

			msl::json status;
			status.set("id",static_cast<int>(static_cast<unsigned char>(id)));
			status.set("serial_port",_serial_port);
			status.set("serial_baud",_serial_baud);
			status.set("serial_status",static_cast<int>(_serial.good()));
			status.set("sd",static_cast<int>(static_cast<unsigned char>(sd)));
			status.set("jpg",static_cast<int>(static_cast<unsigned char>(jpg)));
			status.set("nex",static_cast<int>(static_cast<unsigned char>(nex)));
			status.set("lat",lat);
			status.set("lng",lng);
			status.set("alt",alt);
			status.set("fix",static_cast<int>(fix));
			status.set("course",course);
			status.set("speed",speed);
			response.set("status",status.str());

			msl::json jpg_camera;
			jpg_camera.set("width",160);
			jpg_camera.set("height",120);
			jpg_camera.set("src","/test.jpg");
			response.set("jpg",jpg_camera.str());

			msl::json nex_camera;
			nex_camera.set("size",0);
			response.set("nex",nex_camera.str());

			return response.str();
		}

		void close()
		{
			if(_serial.good())
				_serial.close();
		}

		char id;
		std::string _serial_port;
		unsigned int _serial_baud;
		msl::serial _serial;
		int _packet_state;
		std::string _packet_buffer;
		char sd;
		char jpg;
		char nex;
		float lat;
		float lng;
		float alt;
		char fix;
		char sats;
		float course;
		float speed;
		std::string _jpg_data;
		short _jpg_seq;
};

void service_client(msl::socket& client,const std::string& message);

std::vector<uav> uavs;

int main()
{
	msl::socket server("0.0.0.0:8080");
	std::vector<msl::socket> clients;
	std::vector<std::string> client_messages;
	server.create_tcp();

	if(server.good())
	{
		std::cout<<"socket :)"<<std::endl;
	}
	else
	{
		std::cout<<"socket :("<<std::endl;
		exit(0);
	}

	while(true)
	{
		for(unsigned int ii=0;ii<uavs.size();++ii)
			uavs[ii].update_serial();

		//Check for a Connecting Client
		msl::socket client=server.accept();

		//If Client Connected
		if(client.good())
		{
			clients.push_back(client);
			client_messages.push_back("");
		}

		//Handle Clients
		for(unsigned int ii=0;ii<clients.size();++ii)
		{
			//Service Good Clients
			if(clients[ii].good())
			{
				//Temp
				char byte='\n';

				//Get a Byte
				while(clients[ii].available()>0&&clients[ii].read(&byte,1)==1)
				{
					//Add the Byte to Client Buffer
					client_messages[ii]+=byte;

					//Check for an End Byte
					if(msl::ends_with(client_messages[ii],"\r\n\r\n"))
					{
						service_client(clients[ii],client_messages[ii]);
						client_messages[ii].clear();
					}
				}
			}

			//Disconnect Bad Clients
			else
			{
				clients[ii].close();
				clients.erase(clients.begin()+ii);
				client_messages.erase(client_messages.begin()+ii);
				--ii;
			}
		}

		//Give OS a Break
		usleep(0);
	}

	return 0;
}

void service_client(msl::socket& client,const std::string& message)
{
	//Get Requests
	if(msl::starts_with(message,"GET"))
	{
		//Create Parser
		std::istringstream istr(msl::http_to_ascii(message));

		//Parse the Request
		std::string request;
		istr>>request;
		istr>>request;

		//Web Root Variable (Where your web files are)
		std::string web_root="web";

		//Check for Index
		if(request=="/")
			request+="index.html";

		//Mime Type Variable (Default plain text)
		std::string mime_type="text/plain";

		//Commands Check
		if(msl::starts_with(request,"/&"))
		{
			std::string variable="";
			std::string value="";
			unsigned int state=0;
			char uav_id=0;
			unsigned int uav_index=-1;

			for(unsigned int ii=2;ii<request.size();++ii)
			{
				if(state==0)
				{
					if(request[ii]!='=')
						variable+=request[ii];
					else
						state=1;
				}
				else if(state==1)
				{
					if(request[ii]!='&')
						value+=request[ii];

					if(request[ii]=='&'||ii+1>=request.size())
					{
						if(variable=="add")
						{
							msl::json object(value);

							bool found_id=false;
							bool found_port=false;

							for(unsigned int jj=0;jj<uavs.size();++jj)
							{
								if(uavs[jj].id==static_cast<char>(msl::to_int(object.get("id"))))
								{
									found_id=true;
									break;
								}

								if(uavs[jj]._serial_port==object.get("port"))
								{
									found_port=true;
									break;
								}
							}

							if(found_id||msl::to_int(object.get("id"))==0)
							{
								client<<msl::http_pack_string("invalid id",mime_type);
							}
							else if(found_port)
							{
								client<<msl::http_pack_string("invalid port",mime_type);
							}
							else
							{
								uav temp(static_cast<char>(msl::to_int(object.get("id"))),object.get("port"),msl::to_int(object.get("baud")));
								uavs.push_back(temp);
								client<<msl::http_pack_string("",mime_type);
							}

							break;
						}
						else if(variable=="remove")
						{
							for(unsigned int jj=0;jj<uavs.size();++jj)
							{
								if(static_cast<unsigned char>(uavs[jj].id)==static_cast<unsigned char>(msl::to_int(value)))
								{
									uavs[ii].close();
									uavs.erase(uavs.begin()+jj);
									break;
								}
							}
						}
						else if(variable=="uavs")
						{
							if(msl::to_bool(value))
							{
								msl::json response;
								response.set("size",uavs.size());

								for(unsigned int jj=0;jj<uavs.size();++jj)
									response.set(msl::to_string(jj),static_cast<int>(static_cast<unsigned char>(uavs[jj].id)));

								client<<msl::http_pack_string(response.str(),mime_type);
							}

							break;
						}
						else
						{
							if(variable=="id")
							{
								uav_id=msl::to_int(value);

								for(unsigned int jj=0;jj<uavs.size();++jj)
								{
									if(uavs[jj].id==uav_id)
									{
										uav_index=jj;
										//TEST!
										uavs[uav_index].lat+=0.00001;
										break;
									}
								}
							}
							else if(uav_index!=static_cast<unsigned int>(-1))
							{
								if(variable=="status")
								{
									if(msl::to_bool(value))
									{
										client<<msl::http_pack_string(uavs[uav_index].info(),mime_type);
									}
								}
								else if(variable=="radio")
								{
									uavs[uav_index].change_hw(1,msl::to_bool(value));
								}
								else if(variable=="jpg_camera")
								{
									uavs[uav_index].change_hw(2,msl::to_bool(value));
								}
								else if(variable=="nex_camera")
								{
									uavs[uav_index].change_hw(3,msl::to_bool(value));
								}
							}
						}

						variable="";
						value="";
						state=0;
					}
				}
			}
		}

		//File Requests
		else
		{
			//Check for Code Mime Type
			if(msl::ends_with(request,".js"))
				mime_type="application/x-javascript";

			//Check for Images Mime Type
			else if(msl::ends_with(request,".gif"))
				mime_type="image/gif";
			else if(msl::ends_with(request,".jpeg"))
				mime_type="image/jpeg";
			else if(msl::ends_with(request,".png"))
				mime_type="image/png";
			else if(msl::ends_with(request,".tiff"))
				mime_type="image/tiff";
			else if(msl::ends_with(request,".svg"))
				mime_type="image/svg+xml";
			else if(msl::ends_with(request,".ico"))
				mime_type="image/vnd.microsoft.icon";

			//Check for Text Mime Type
			else if(msl::ends_with(request,".css"))
				mime_type="text/css";
			else if(msl::ends_with(request,".htm")||msl::ends_with(request,".html"))
				mime_type="text/html";

			//File Data Variable
			std::string file;

			//Load File
			if(msl::file_to_string(web_root+request,file))
				client<<msl::http_pack_string(file,mime_type);

			//Bad File
			else if(msl::file_to_string(web_root+"/not_found.html",file))
				client<<msl::http_pack_string(file);
		}

		//Close Connection
		client.close();
	}

	//Other Requests (Just kill connection...it's either hackers or idiots...)
	else
	{
		client.close();
	}
}