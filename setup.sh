#!/bin/bash

# Stop on error
set -e

#SETUP tools
if [[ ! -d "tools/" ]]; then
	mkdir -p tools
	cd tools

	curl -L https://github.com/arduino/arduino-cli/releases/download/v1.1.1/arduino-cli_1.1.1_Windows_64bit.zip > arduino-cli.zip
	unzip arduino-cli.zip arduino-cli.exe
	rm arduino-cli.zip

	./arduino-cli core install esp32:esp32 --config-file ../.cli-config.yml

	cd ..
fi

#SETUP libraries
if [[ ! -d "libraries/" ]]; then
	mkdir -p libraries
	cd libraries

	curl -L https://github.com/adafruit/Adafruit_CAN/archive/refs/tags/0.2.1.zip > ar.zip
	unzip ar.zip
	rm ar.zip

	cd ..
fi
