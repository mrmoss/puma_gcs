g++ ../src/main.cpp ../src/drone.cpp ../src/location.cpp ../src/msl/file_util.cpp ../src/msl/serial.cpp ../src/msl/socket.cpp ../src/msl/socket_util.cpp ../src/msl/string_util.cpp ../src/msl/time_util.cpp -o puma_gcs -Wall -O
#wine ~/.wine/drive_c/Program\ Files/CodeBlocks/MinGW/bin/g++.exe ../src/main.cpp ../src/drone.cpp ../src/location.cpp ../src/msl/file_util.cpp ../src/msl/serial.cpp ../src/msl/socket.cpp ../src/msl/socket_util.cpp ../src/msl/string_util.cpp ../src/msl/time_util.cpp -o puma_gcs.exe -Wall -O -lwsock32 -Wno-unknown-pragmas