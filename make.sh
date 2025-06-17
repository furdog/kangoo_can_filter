#!/bin/bash

###############################################################################
# CONFIGURATION:
###############################################################################
COMPILER=./tools/arduino-cli

SERIAL_PORT=COM3
MONITOR_BAUD=115200
OTA_IP="7.7.7.7"

# Targets:
export TARGET=can_filter_v1_native_esp32
#export CANLIB_VARIANT=CAN_FILTER_CAN_ADAFRUIT
export CANLIB_VARIANT=CAN_FILTER_CAN_ACAN

#export  TARGET=can_filter_v2_native_esp32c6

#EXTRA_FLAGS="-v"

###############################################################################
# TARGETS AVAILABLE
###############################################################################
# enumerate (and empty) all generated files
> target.gen.h
> index.gen.h

if [ "$TARGET" == "can_filter_v1_native_esp32" ]; then
	BOARD=esp32:esp32:nodemcu-32s
	FQBN=:UploadSpeed=921600
	PROPS='--build-property build.partitions=partitions
	       --build-property upload.maximum_size=1638400'
	echo "$BOARD"
	echo "#define CAN_FILTER_V1_NATIVE_ESP32" > target.gen.h
	echo "#define" "$CANLIB_VARIANT" >> target.gen.h
elif [ "$TARGET" == "can_filter_v2_native_esp32c6" ]; then
	BOARD=esp32:esp32:esp32c6
	FQBN=:CDCOnBoot=cdc
else
	echo "Bad target!"
	exit 1
fi

###############################################################################
# MAIN
###############################################################################
# Set environment variables
if [ -z "${GIT_REPO_VERSION+x}" ]; then
	export GIT_REPO_VERSION=$(git describe --tags)
fi

mk_base64_updater()
{
	local BOARD_PATH="${BOARD//:/\.}"  # Replace ':' with '.'
	local FILE="build/${BOARD_PATH}/$(basename "$PWD").ino.bin"

	# Generate BASE64 firmware
	export BASE64_ENCODED_FIRMWARE=$(base64 -w 0 "$FILE")

	# Generate updater html file (with embedded base64 firmware)
	awk/ENV.awk web/kangoo_can_filter_updater.html > \
		     build/"$BOARD_PATH"/"$TARGET"_"$GIT_REPO_VERSION".html
}

compile() {
	# Setup tools and libraries
	./setup.sh
	if [[ $? -ne 0 ]]; then
	    echo "FATAL ERROR: Setup failed."
	    exit 1
	fi

	# Replace environment variables inside web page
	awk/ENV.awk web/index.html > index.gen.html
	
	# Compress WEB
	gzip -9 -c index.gen.html > index.gen.html.gz

	# Make C array from WEB
	xxd -i index.gen.html.gz > index.gen.h

	echo "PROPS: " ${PROPS}
	echo "FQBN: " ${FQBN}
	while ! ${COMPILER} compile -b ${BOARD}${FQBN} --warnings "all" \
			   ${PROPS} -e --libraries "libraries/" \
			   ${EXTRA_FLAGS}; do

		read -p "Press any key to continue "
		exit
	done

	mk_base64_updater # Make base64 firmware
}

# Function to monitor
monitor() {
	if [ -n "${SERIAL_PORT+x}" ]; then
		while true; do
			${COMPILER} monitor -p ${SERIAL_PORT} \
			      --config baudrate=${MONITOR_BAUD} ${EXTRA_FLAGS};
			sleep 1
		done
	fi
}

upload() {
	if [ -n "${SERIAL_PORT+x}" ]; then
		while ! ${COMPILER} upload -b ${BOARD}${FQBN} \
				   -p ${SERIAL_PORT} ${EXTRA_FLAGS}; do
			sleep 1
		done
	fi
}

web_upload() {
	local BOARD_PATH="${BOARD//:/\.}"  # Replace ':' with '.'
	local FILE="build/${BOARD_PATH}/$(basename "$PWD").ino.bin"

	if [ ! -f "$FILE" ]; then
		echo "Firmware file not found: $FILE"
		return 1
	fi

	while true; do
		echo "Uploading $FILE to http://${OTA_IP}/update..."
		curl -F "file=@$FILE" http://${OTA_IP}/update && break
		echo "Upload failed, retrying in 1 second..."
		sleep 1
	done

	echo "Upload complete."
}

# Check if the argument is "monitor"
if [ "$1" == "compile" ]; then
	compile
elif [ "$1" == "build" ]; then
	compile
elif [ "$1" == "monitor" ]; then
	monitor
elif [ "$1" == "upload" ]; then
	upload
elif [ "$1" == "flash" ]; then
	upload
elif [[ "$1" == "web" && ( "$2" == "upload" || "$2" == "flash" ) ]]; then
	web_upload
elif [ "$1" == "web" ]; then
	compile
	web_upload
else
	compile
	upload
	monitor
fi

read -p "Press any key to continue "
