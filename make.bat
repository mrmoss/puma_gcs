cd g++/bin
g++.exe ../../src/main.cpp ../../src/location.cpp ../../src/crc.cpp ../../src/uav.cpp ../../src/msl/file_util.cpp ../../src/msl/json.cpp ../../src/msl/serial.cpp ../../src/msl/socket.cpp ../../src/msl/socket_util.cpp ../../src/msl/string_util.cpp ../../src/msl/time_util.cpp -o ../../bin/gcs.exe -Wall -O -lwsock32 -Wno-unknown-pragmas
cd ../../