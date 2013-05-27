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

//JSON Header
#include "msl/json.hpp"

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
	drones.push_back(drone(1,"/dev/ttyACM0",57600));

	//FOR TESTING SERVER TO WEB TX
	/*std::string temp_packet_test="";
	temp_packet_test+=(char)0x01;
	temp_packet_test+=(char)0x00;
	float temp_lat_test=0.123;
	float temp_lng_test=0.456;
	float temp_alt_test=0.789;
	for(unsigned int ii=0;ii<4;++ii)
		temp_packet_test+=*(char*)((&temp_lat_test)+ii);
	for(unsigned int ii=0;ii<4;++ii)
		temp_packet_test+=*(char*)((&temp_lng_test)+ii);
	for(unsigned int ii=0;ii<4;++ii)
		temp_packet_test+=*(char*)((&temp_alt_test)+ii);

	drones[0].img2_add_position(temp_packet_test);*/

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

			//Find the Drone
			unsigned int uav_index=-1;

			for(unsigned int ii=0;ii<drones.size();++ii)
			{
				if(drones[ii].id()==uav_id)
				{
					uav_index=ii;
					break;
				}
			}

			//Send Everything
			if(!parsed_id)
				client<<msl::http_pack_string("Not implemented yet!","text/plain");

			//Check for Valid ID (1-255, 0 is invalid!) and Valid Index (> -1)
			else if(uav_id>0&&uav_id<256&&uav_index!=static_cast<unsigned int>(-1))
			{
				//Make UAV JSON Object
				msl::json uav_json;
				msl::json uav_json_stat;
					msl::json uav_json_position;
						uav_json_position.set("lat",drones[uav_index].position().lat);
						uav_json_position.set("lng",drones[uav_index].position().lng);
						uav_json_position.set("alt",drones[uav_index].position().alt);
					uav_json_stat.set("pos",uav_json_position.str());
					msl::json uav_json_radio;
						uav_json_radio.set("io",drones[uav_index].stat_get()&1);
						uav_json_radio.set("quality",100);
					uav_json_stat.set("radio",uav_json_radio.str());
					msl::json uav_json_camera;
						uav_json_camera.set("io",drones[uav_index].stat_get()&2);
						uav_json_camera.set("debug","ok");
						uav_json_camera.set("angle",drones[uav_index].img2_angle());
						uav_json_camera.set("servo",drones[uav_index].img2_servo());
					uav_json_stat.set("camera",uav_json_camera.str());
				msl::json uav_json_img;
					uav_json_img.set("size",drones[uav_index].img2_map().size());

					for(std::map<short,location>::const_iterator iter=drones[uav_index].img2_map().begin();iter!=drones[uav_index].img2_map().end();++iter)
					{
						msl::json uav_json_img_num;
							uav_json_img_num.set("src","test");
							msl::json uav_json_loc;
								uav_json_loc.set("lat",msl::to_string(iter->second.lat));
								uav_json_loc.set("lng",msl::to_string(iter->second.lng));
								uav_json_loc.set("alt",msl::to_string(iter->second.alt));
							uav_json_img_num.set("pos",uav_json_loc.str());
						uav_json_img.set(msl::to_string(iter->first),uav_json_img_num.str());
					}

				uav_json.set("stat",uav_json_stat.str());
				uav_json.set("img",uav_json_img.str());

				//Send Whole UAV Object
				if(!(command_parser>>request))
					client<<msl::http_pack_string(uav_json.str(),"text/plain");

				//Status Request
				else if(request=="stat")
				{
					//Send Whole Status Object
					if(!(command_parser>>request))
						client<<msl::http_pack_string(uav_json.get("stat"),"text/plain");

					//Position Request
					else if(request=="pos")
					{
						//Send Whole Position Object
						if(!(command_parser>>request))
							client<<msl::http_pack_string(uav_json_stat.get("pos"),"text/plain");

						//Send Lattitude
						else if(request=="lat")
							client<<msl::http_pack_string(uav_json_position.get("lat"),"text/plain");

						//Send Longitude
						else if(request=="lng")
							client<<msl::http_pack_string(uav_json_position.get("lng"),"text/plain");

						//Send Altitude
						else if(request=="alt")
							client<<msl::http_pack_string(uav_json_position.get("alt"),"text/plain");
					}

					//Radio Request
					else if(request=="radio")
					{
						//Send Whole Radio Object
						if(!(command_parser>>request))
							client<<msl::http_pack_string(uav_json_radio.str(),"text/plain");

						//IO Request
						else if(request=="io")
						{
							//Send Current Radio IO State
							if(!(command_parser>>request))
								client<<msl::http_pack_string(uav_json_radio.get("io"),"text/plain");

							//Set Current Radio IO State
							else
							{
								if(msl::to_int(request)==0)
									drones[uav_index].stat_set(drones[uav_index].stat_get()&(~1));
								else
									drones[uav_index].stat_set(drones[uav_index].stat_get()|1);
							}
						}

						//Send Radio Quality
						else if(request=="quality")
							client<<msl::http_pack_string(uav_json_radio.get("quality"),"text/plain");
					}

					//Camera Request
					else if(request=="camera")
					{
						//Send Whole Camera Object
						if(!(command_parser>>request))
							client<<msl::http_pack_string(uav_json_camera.str(),"text/plain");

						//IO Request
						else if(request=="io")
						{
							//Send Current Camera IO State
							if(!(command_parser>>request))
								client<<msl::http_pack_string(uav_json_camera.get("io"),"text/plain");

							//Set Current Camera IO State
							else
							{
								if(msl::to_int(request)==0)
									drones[uav_index].stat_set(drones[uav_index].stat_get()&(~2));
								else
									drones[uav_index].stat_set(drones[uav_index].stat_get()|2);
							}
						}

						//Send Camera Debug
						else if(request=="debug")
							client<<msl::http_pack_string(uav_json_camera.get("debug"),"text/plain");

						//Send Camera Angle
						else if(request=="angle")
							client<<msl::http_pack_string(uav_json_camera.get("angle"),"text/plain");

						//Send Camera Servo Angle
						else if(request=="servo")
							client<<msl::http_pack_string(uav_json_camera.get("servo"),"text/plain");
					}
				}

				//Image Request
				else if(request=="img")
				{
					//Parse Image Number
					bool parsed=(command_parser>>request);
					std::string uav_img=request;

					//Send Whole Image Object
					if(!parsed)
						client<<msl::http_pack_string(uav_json_img.str(),"text/plain");

					//Send Total Number of Images
					else if(request=="size")
						client<<msl::http_pack_string(uav_json_img.get("size"),"text/plain");

					//Check for Valid Image Number (1-32676, 0 is invalid!)
					else if(msl::to_int(uav_img)>0)
					{
						//Get Image Number JSON Object
						msl::json uav_json_img_number(uav_json_img.get(uav_img));

						//Send Whole Image Number Object
						if(!(command_parser>>request))
							client<<msl::http_pack_string(uav_json_img.get(uav_img),"text/plain");

						//Position Request
						else if(request=="pos")
						{
							//Get Position JSON Object
							msl::json uav_json_img_number_pos(uav_json_img_number.get("pos"));

							//Send Whole Image Position Object
							if(!(command_parser>>request))
								client<<msl::http_pack_string(uav_json_img_number_pos.str(),"text/plain");

							//Send Image Lattitude
							else if(request=="lat")
								client<<msl::http_pack_string(uav_json_img_number_pos.get("lat"),"text/plain");

							//Send Image Longitude
							else if(request=="lng")
								client<<msl::http_pack_string(uav_json_img_number_pos.get("lng"),"text/plain");

							//Send Image Altitude
							else if(request=="alt")
								client<<msl::http_pack_string(uav_json_img_number_pos.get("alt"),"text/plain");
						}

						//Source Request
						else if(request=="src")
							client<<msl::http_pack_string(uav_json_img_number.get("src"),"text/plain");
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