SRC=src/main.cpp src/location.cpp src/crc.cpp src/uav.cpp src/msl/file_util.cpp src/msl/json.cpp src/msl/serial.cpp src/msl/serial_util.cpp src/msl/socket.cpp src/msl/socket_util.cpp src/msl/string_util.cpp src/msl/time_util.cpp
OPTS=-O
CFLAGS=-Wall $(OPTS)
COMPILER=g++
OUT=bin

all: gcs

gcs: $(SRC)
	$(COMPILER) $(CFLAGS) $(SRC) -o $(OUT)/$@

clean:
	rm -f $(OUT)/gcs-linux64