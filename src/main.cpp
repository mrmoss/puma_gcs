//Puma GCS
//	Created By:		Steven Kibler and Mike Moss
//	Modified On:	05/22/2013

//File Utility Header
#include "msl/file_util.hpp"

//IO Stream Header
#include <iostream>

//Serial Header
#include "msl/serial.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//String Header
#include <string>

//String Utility Header
#include "msl/string_util.hpp"

//Vector Header
#include <vector>

//Service Client Function Declaration
void service_client(msl::socket& client,const std::string& message);

class location
{
	public:
		location(const float lattitude=0,const float longitude=0,const float altitude=0):lat(lattitude),lng(longitude),alt(altitude)
		{}

		float lat;
		float lng;
		float alt;
};

//UAV Class
class uav
{
	public:
		bool set_stat(const std::string& data)
		{
			if(data.size()==17)
			{
				flags=data[0];
				camera_angle=*(short*)(data.c_str()+1);
				camera_servo=*(short*)(data.c_str()+3);
				loc.lat=*(float*)(data.c_str()+5);
				loc.lng=*(float*)(data.c_str()+9);
				loc.alt=*(float*)(data.c_str()+13);
				return true;
			}

			return false;
		}

		bool add_location(const std::string& data)
		{
			if(data.size()==12)
			{
				location temp(*(short*)(data.c_str()),*(short*)(data.c_str()+4),*(short*)(data.c_str()+8));
				nex_locs.push_back(temp);
			}
		}

		char flags;
		short camera_angle;
		short camera_servo;
		location loc;
		std::vector<location> nex_locs;
};

uav MY_UAV;

//Main
int main()
{
	//Create Serial
	msl::serial serial("/dev/ttyUSB0",57600);
	serial.connect();

	//Serial Parsing Data
	std::string serial_buffer="";
	int serial_state=0;

	//Check Serial
	if(serial.good())
	{
		std::cout<<"Serial =)"<<std::endl;
	}
	else
	{
		std::cout<<"Serial =("<<std::endl;
		exit(0);
	}

		//Create Server
	msl::socket server("0.0.0.0:8080");
	server.create_tcp();

	//Check Server
	if(server.good())
	{
		std::cout<<"Socket =)"<<std::endl;
	}
	else
	{
		std::cout<<"Socket =("<<std::endl;
		exit(0);
	}

	//Vectors for Clients
	std::vector<msl::socket> web_clients;
	std::vector<std::string> web_buffers;

	//Be a server...forever...
	while(true)
	{
		//Temp Byte
		char byte='\n';

		//Check for Serial Data
		if(serial.available()>0&&serial.read(&byte,1)==1)
		{
			if(serial_state==0)
			{
				if(byte=='s')
					serial_state=1;
				else if(byte=='c')
					serial_state=4;
			}
			else if(serial_state==1)
			{
				if(byte=='t')
					serial_state=2;
				else
					serial_state=0;
			}
			else if(serial_state==2)
			{
				if(byte=='a')
					serial_state=3;
				else
					serial_state=0;
			}
			else if(serial_state==3)
			{
				if(byte=='t')
					serial_state=7;
			}
			else if(serial_state==4)
			{
				if(byte=='a')
					serial_state=5;
				else
					serial_state=0;
			}
			else if(serial_state==5)
			{
				if(byte=='m')
					serial_state=6;
				else
					serial_state=0;
			}
			else if(serial_state==6)
			{
				if(byte=='1')
					serial_state=8;
				else if(byte=='2')
					serial_state=9;
				else
					serial_state=0;
			}
			else if(serial_state==7)
			{
				if(serial_buffer.size()<14)
				{
					serial_buffer+=byte;
				}
				else
				{
					MY_UAV.set_stat(serial_buffer);
					serial_state=0;
				}
			}
			else if(serial_state==8)
			{
				if(serial_buffer.size()<2||(serial_buffer.size()>=2&&serial_buffer.size()-2<*(short*)(serial_buffer.c_str())))
				{
					serial_buffer+=byte;
				}
				else
				{
					//save the image
					serial_state=0;
				}
			}
			else if(serial_state==9)
			{
				if(serial_buffer.size()<12)
				{
					serial_buffer+=byte;
				}
				else
				{
					//Set the variables
					serial_state=0;
				}
			}
		}

		//Check for a Connecting Client
		msl::socket client=server.accept();

		//If Client Connected
		if(client.good())
		{
			web_clients.push_back(client);
			web_buffers.push_back("");
		}

		//Handle Clients
		for(unsigned int ii=0;ii<web_clients.size();++ii)
		{
			//Service Good Clients
			if(web_clients[ii].good())
			{
				//Get a Byte
				while(web_clients[ii].available()>0&&web_clients[ii].read(&byte,1)==1)
				{
					//Add the Byte to Client Buffer
					web_buffers[ii]+=byte;

					//Check for an End Byte
					if(msl::ends_with(web_buffers[ii],"\r\n\r\n"))
					{
						service_client(web_clients[ii],web_buffers[ii]);
						web_buffers[ii].clear();
					}
				}
			}

			//Disconnect Bad Clients
			else
			{
				web_clients[ii].close();
				web_clients.erase(web_clients.begin()+ii);
				web_buffers.erase(web_buffers.begin()+ii);
				--ii;
			}
		}
	}

	//Call Me Plz T_T
	return 0;
}

//Service Client Function Definition
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
			request="/index.html";

		//Commands
		if(msl::starts_with(request,"/uav"))
		{
			//Change '/'s to ' 's
			for(unsigned int ii=0;ii<request.size();++ii)
				if(request[ii]=='/')
					request[ii]=' ';

			//Create Parser
			std::istringstream command_parser(request);

			//Parse "uav"
			command_parser>>request;

			//Parse id
			bool parsed_uav=(command_parser>>request);
			int uav_id=msl::to_int(request);

			if(!parsed_uav)
			{
				std::cout<<request<<std::endl;
			}
			else if(uav_id>0)
			{
				if(!(command_parser>>request))
				{
					std::cout<<request<<std::endl;
				}
				else if(request=="stat")
				{
					if(!(command_parser>>request))
					{
						std::cout<<request<<std::endl;
					}
					else if(request=="lat")
					{
						std::cout<<request<<std::endl;
					}
					else if(request=="lng")
					{
						std::cout<<request<<std::endl;
					}
					else if(request=="alt")
					{
						std::cout<<request<<std::endl;
					}
					else if(request=="radio")
					{
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="io")
						{
							if(!(command_parser>>request))
							{
								std::cout<<request<<" get"<<std::endl;
							}
							else
							{
								std::cout<<request<<" set"<<std::endl;
							}
						}
						else if(request=="quality")
						{
							std::cout<<request<<std::endl;
						}
					}
					else if(request=="camera")
					{
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="io")
						{
							if(!(command_parser>>request))
							{
								std::cout<<request<<" get"<<std::endl;
							}
							else
							{
								std::cout<<request<<" set"<<std::endl;
							}
						}
						else if(request=="debug")
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="angle")
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="servo")
						{
							std::cout<<request<<std::endl;
						}
					}
				}
				else if(request=="img")
				{
					bool parsed=(command_parser>>request);
					int uav_img=msl::to_int(request);

					if(!parsed)
					{
						std::cout<<request<<std::endl;
					}
					else if(request=="size")
					{
						std::cout<<request<<std::endl;
					}
					else if(uav_img>0)
					{
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="src")
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="lat")
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="lng")
						{
							std::cout<<request<<std::endl;
						}
						else if(request=="alt")
						{
							std::cout<<request<<std::endl;
						}
					}
				}
			}
		}

		//Web Pages
		else
		{
			//Mime Type Variable (Default plain text)
			std::string mime_type="text/plain";

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

		//Close Connectiona
		client.close();
	}

	//Other Requests (Just kill connection...it's either hackers or idiots...)
	else
	{
		client.close();
	}
}