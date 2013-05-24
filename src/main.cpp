//Puma GCS Source
//	Created By:		Steven Kibler and Mike Moss
//	Modified On:	05/22/2013

//C Standard Library Header
#include <cstdlib>

//Drone Header
#include "drone.hpp"

//File Utility Header
#include "msl/file_util.hpp"

//IO Stream Header
#include <iostream>

//Location Header
#include "location.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//String Header
#include <string>

//String Utility Header
#include "msl/string_util.hpp"

//Time Utility Header
#include "msl/time_util.hpp"

//Vector Header
#include <vector>

//TTD
//		Make socket responses send actual data.
//		Create serial send functions.
//		Make better names for drone class members.
//		Comment everything that isn't commented.
//		Test out everything.

//Global Drone Vector
std::vector<drone> drones;

//Service Client Function Declaration
void service_client(msl::socket& client,const std::string& message);

//Main
int main()
{
	//Create a Drone
	drones.push_back(drone(1,"/dev/ttyUSB0",57600));

	//Connect and Check Drones
	for(unsigned int ii=0;ii<drones.size();++ii)
	{
		drones[ii].connect();

		std::cout<<"Drone["<<static_cast<unsigned int>(drones[ii].id())<<"] ";

		if(drones[ii].good())
		{
			std::cout<<"=)"<<std::endl;
		}
		else
		{
			std::cout<<"=("<<std::endl;
			exit(0);
		}
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
		//Update Drones
		for(unsigned int ii=0;ii<drones.size();++ii)
			drones[ii].update();

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
			//Temp Byte
			char byte='\n';

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

		//Give OS a Break
		usleep(0);
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
			bool parsed_id=(command_parser>>request);
			int uav_id=msl::to_int(request);

			//Send Everything
			if(!parsed_id)
			{
				std::cout<<request<<std::endl;
			}

			//Check for Valid ID (1-255, 0 is invalid!)
			else if(uav_id>0&&uav_id<256)
			{
				//Send Whole UAV Object
				if(!(command_parser>>request))
				{
					std::cout<<request<<std::endl;
				}

				//Status Request
				else if(request=="stat")
				{
					//Send Whole Status Object
					if(!(command_parser>>request))
					{
						std::cout<<request<<std::endl;
					}

					//Send Current Lattitude
					else if(request=="lat")
					{
						std::cout<<request<<std::endl;
					}

					//Send Current Longitude
					else if(request=="lng")
					{
						std::cout<<request<<std::endl;
					}

					//Send Current Altitude
					else if(request=="alt")
					{
						std::cout<<request<<std::endl;
					}

					//Radio Request
					else if(request=="radio")
					{
						//Send Whole Radio Object
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}

						//IO Request
						else if(request=="io")
						{
							//Send Current Radio IO State
							if(!(command_parser>>request))
							{
								std::cout<<request<<" get"<<std::endl;
							}

							//Set Current Radio IO State
							else
							{
								std::cout<<request<<" set"<<std::endl;
							}
						}

						//Send Radio Quality
						else if(request=="quality")
						{
							std::cout<<request<<std::endl;
						}
					}

					//Camera Request
					else if(request=="camera")
					{
						//Send Whole Camera Object
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}

						//IO Request
						else if(request=="io")
						{
							//Send Current Camera IO State
							if(!(command_parser>>request))
							{
								std::cout<<request<<" get"<<std::endl;
							}

							//Set Current Camera IO State
							else
							{
								std::cout<<request<<" set"<<std::endl;
							}
						}

						//Send Camera Debug
						else if(request=="debug")
						{
							std::cout<<request<<std::endl;
						}

						//Send Camera Angle
						else if(request=="angle")
						{
							std::cout<<request<<std::endl;
						}

						//Send Camera Servo Angle
						else if(request=="servo")
						{
							std::cout<<request<<std::endl;
						}
					}
				}

				//Image Request
				else if(request=="img")
				{
					//Parse Image Number
					bool parsed=(command_parser>>request);
					int uav_img=msl::to_int(request);

					//Send Whole Image Object
					if(!parsed)
					{
						std::cout<<request<<std::endl;
					}

					//Send Total Number of Images
					else if(request=="size")
					{
						std::cout<<request<<std::endl;
					}

					//Check for Valid Image Number (1-32676, 0 is invalid!)
					else if(uav_img>0)
					{

						//Send Whole Image Number Object
						if(!(command_parser>>request))
						{
							std::cout<<request<<std::endl;
						}

						//Send Image Source
						else if(request=="src")
						{
							std::cout<<request<<std::endl;
						}

						//Send Image Lattitude
						else if(request=="lat")
						{
							std::cout<<request<<std::endl;
						}

						//Send Image Longitude
						else if(request=="lng")
						{
							std::cout<<request<<std::endl;
						}

						//Send Image Altitude
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

		//Close Connection
		client.close();
	}

	//Other Requests (Just kill connection...it's either hackers or idiots...)
	else
	{
		client.close();
	}
}