#!/bin/bash

SERIAL_PORT=COM26
MONITOR_BAUD=115200

#Which hardware to use
#BOARD=esp32:esp32:nodemcu-32s
#BOARD=esp8266:esp8266:nodemcu
#BOARD=esp32:esp32:nodemcu-32s
BOARD=esp32:esp32:esp32c6

FQBN=:UploadSpeed=921600

COMPILER=./tools/arduino-cli

if [[ "$BOARD" == "esp8266:esp8266:nodemcu" ]]; then
	FQBN=
	PROPS=
	echo "$BOARD"
elif [[ "$BOARD" == "esp32:esp32:nodemcu-32s" ]]; then
	PROPS='--build-property build.partitions=partitions
	       --build-property upload.maximum_size=1638400'
	echo "$BOARD"
elif [[ "$BOARD" == "esp32:esp32:esp32c6" ]]; then
	FQBN=:CDCOnBoot=cdc
fi

# Function to compile and upload
compile_and_upload() {
	# Setup tools and libraries
	./setup.sh
	if [[ $? -ne 0 ]]; then
	    echo "FATAL ERROR: Setup failed."
	    exit 1
	fi
	
	# Compress WEB
	gzip -9 -c index.html > index.html.gz

	# Make C array from WEB
	xxd -i index.html.gz > index.h

	echo "Web interface has been built successfuly!"


	echo "PROPS: " ${PROPS}
	echo "FQBN: " ${FQBN}
	while ! ${COMPILER} compile -b ${BOARD}${FQBN} --warnings "all" ${PROPS} -e --libraries "libraries/" -v; do
		read -p "Press any key to continue "
		exit
	done
    
	if [ -n "${SERIAL_PORT+x}" ]; then
		while ! ${COMPILER} upload -b ${BOARD}${FQBN} -p ${SERIAL_PORT} -v; do
			sleep 1
		done
	fi
}

# Function to monitor
monitor() {
	if [ -n "${SERIAL_PORT+x}" ]; then
		while true; do
			${COMPILER} monitor -p ${SERIAL_PORT} --config baudrate=${MONITOR_BAUD} -v;
			sleep 1
		done
	fi
}

upload() {
	if [ -n "${SERIAL_PORT+x}" ]; then
		while ! ${COMPILER} upload -b ${BOARD}${FQBN} -p ${SERIAL_PORT} -v; do
			sleep 1
		done
	fi
}

# Check if the argument is "monitor"
if [ "$1" == "monitor" ]; then
	monitor
elif [ "$1" == "upload" ]; then
	upload
elif [ "$1" == "flash" ]; then
	upload
else
	compile_and_upload
	monitor
fi

read -p "Press any key to continue "
