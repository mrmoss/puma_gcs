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

//Main
int main()
{
	//Create Server
	msl::socket server("0.0.0.0:8080");
	server.create_tcp();

	//Check Server
	if(server.good())
		std::cout<<"=)"<<std::endl;
	else
		std::cout<<"=("<<std::endl;

	//Vectors for Clients
	std::vector<msl::socket> clients;
	std::vector<std::string> client_messages;

	//Be a server...forever...
	while(true)
	{
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