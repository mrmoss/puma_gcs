//GCS Source
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//File Utility Header
#include "msl/file_util.hpp"

//IO Stream Header
#include <iostream>

//JSON Header
#include "msl/json.hpp"

//Serial Utility Header
#include "msl/serial_util.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//String Header
#include <string>

//Time Utility Header
#include "msl/time_util.hpp"

//UAV Header
#include "uav.hpp"

//Vector Header
#include <vector>

//Service Client Function Declaration (Services Socket Clients)
void service_client(msl::socket& client,const std::string& message);

//Global UAVs Vector
std::vector<uav> uavs;

//Main
int main(int argc, char* argv[])
{
	//Default Port is 8080
	std::string server_port="8080";

	uavs.push_back(uav(5,"/dev/ttyUSB0",57600));

	//Get Command Line Port
	if(argc>1)
		server_port=argv[1];

	//Create Server
	msl::socket server("0.0.0.0:"+server_port);

	//Server Client Variables
	std::vector<msl::socket> clients;
	std::vector<std::string> client_messages;

	//Start Server
	server.create_tcp();

	//Print Server Status
	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	//Run Server
	while(server.good())
	{
		//Update UAVs
		for(unsigned int ii=0;ii<uavs.size();++ii)
			uavs[ii].update();

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

	//Return
	return 0;
}

//Service Client Function Definition (Services Socket Clients)
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
			//Parsing Variables
			std::string variable="";
			std::string value="";
			unsigned int state=0;
			unsigned char uav_id=0;
			unsigned int uav_index=-1;

			//Parse the Request (Ignore /& at beginning)
			for(unsigned int ii=2;ii<request.size();++ii)
			{
				//Look for Variable
				if(state==0)
				{
					//Looking for '=' Sign as Terminator
					if(request[ii]!='=')
						variable+=request[ii];

					//Terminator Found, Find Value
					else
						state=1;
				}

				//Look for Value
				else if(state==1)
				{
					//Looking for '&' Sign as Terminator
					if(request[ii]!='&')
						value+=request[ii];

					//Terminator Found, Determine What to Do With Variable and Value
					if(request[ii]=='&'||ii+1>=request.size())
					{
						//Add UAV Command
						if(variable=="add")
						{
							//Create JSON Object of Value
							msl::json object(value);

							//Serial Port Checking Variables
							bool found_id=false;
							bool found_port=false;

							//Go Through Existing UAVs
							for(unsigned int jj=0;jj<uavs.size();++jj)
							{
								//Multiple ID Found
								if(uavs[jj].id()==static_cast<unsigned char>(msl::to_int(object.get("id"))))
								{
									found_id=true;
									break;
								}

								//Multiple Serial Port Found
								if(uavs[jj].serial_name()==object.get("port"))
								{
									found_port=true;
									break;
								}
							}

							//Invalid IDs
							if(found_id||msl::to_int(object.get("id"))<=0||msl::to_int(object.get("id"))>255)
							{
								client<<msl::http_pack_string("invalid id",mime_type);
							}

							//Invalid Serial Port
							else if(found_port)
							{
								client<<msl::http_pack_string("invalid port",mime_type);
							}

							//Good ID
							else
							{
								//Create Temporary UAV
								uav temp(msl::to_int(object.get("id")),object.get("port"),msl::to_int(object.get("baud")));

								//Add Temp UAV
								uavs.push_back(temp);
							}

							//Adding an UAV Closes the Message Stream
							break;
						}

						//Remove UAV Command
						else if(variable=="remove")
						{
							//Look for UAV
							for(unsigned int jj=0;jj<uavs.size();++jj)
							{
								//If UAV is Found
								if(static_cast<unsigned char>(uavs[jj].id())==static_cast<unsigned char>(msl::to_int(value)))
								{
									//Close UAV Serial Port
									uavs[ii].close();

									//Remove UAV Object
									uavs.erase(uavs.begin()+jj);

									//Break Out of Loop
									break;
								}
							}

							//Removing an UAV Closes the Message Stream
							break;
						}

						//Serials Request (Get list of valid serial ports)
						else if(variable=="serials")
						{
							//If Value is True
							if(msl::to_bool(value))
							{
								//Get Serial Port Listing
								std::vector<std::string> serial_list=msl::list_serial_ports();

								//Create Response JSON
								msl::json response;

								//Add Serial Port Listing Size to Response
								response.set("size",serial_list.size());

								//Add Serial Port Listings to Response
								for(unsigned int jj=0;jj<serial_list.size();++jj)
									response.set(msl::to_string(jj),serial_list[jj]);

								//Send Response
								client<<msl::http_pack_string(response.str(),mime_type);
							}

							//Getting Serials Closes the Message Stream
							break;
						}

						//UAVs Request (Get Number of UAVs and their corresponding IDs)
						else if(variable=="uavs")
						{
							//If Value is True
							if(msl::to_bool(value))
							{
								//Create Response JSON
								msl::json response;

								//Add UAV Size to Response
								response.set("size",uavs.size());

								//Add UAV IDs to Response
								for(unsigned int jj=0;jj<uavs.size();++jj)
									response.set(msl::to_string(jj),static_cast<unsigned int>(uavs[jj].id()));

								//Send Response
								client<<msl::http_pack_string(response.str(),mime_type);
							}

							//Getting UAVs Closes the Message Stream
							break;
						}

						//Hardware or Status Commands
						else
						{
							//If ID
							if(variable=="id")
							{
								//Set the ID
								uav_id=msl::to_int(value);

								//Look For Index of ID
								for(unsigned int jj=0;jj<uavs.size();++jj)
								{
									//If Matching IDs
									if(uavs[jj].id()==uav_id)
									{
										//Set UAV Index and Break Loop
										uav_index=jj;
										break;
									}
								}
							}

							//If Valid Index
							else if(uav_index!=static_cast<unsigned int>(-1))
							{
								//Status Request
								if(variable=="status")
								{
									//If Value is True Send Request
									if(msl::to_bool(value))
										client<<msl::http_pack_string(uavs[uav_index].json(),mime_type);

									//Status Requests Close the Message Stream
									break;
								}

								//Connect Command
								else if(variable=="serial")
								{
									if(msl::to_bool(value))
										uavs[uav_index].connect();
									else
										uavs[uav_index].close();
								}

								//Radio Command
								else if(variable=="radio")
								{
									uavs[uav_index].set_hw(1,msl::to_bool(value));
								}

								//JPG Camera Command
								else if(variable=="jpg_camera")
								{
									uavs[uav_index].set_hw(2,msl::to_bool(value));
								}

								//NEX Camera Command
								else if(variable=="nex_camera")
								{
									uavs[uav_index].set_hw(3,msl::to_bool(value));
								}
							}
						}

						//Reset Parsing Variables
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
			//Check for ?
			for(unsigned int ii=0;ii<request.size();++ii)
			{
				if(request[ii]=='?')
				{
					request=request.substr(0,ii);
					break;
				}
			}

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
			if(msl::file_to_string(web_root+request,file,true))
				client<<msl::http_pack_string(file,mime_type);

			//Bad File
			else if(msl::file_to_string(web_root+"/not_found.html",file,true))
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