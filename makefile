CC := arduino-cli
BOARD := esp32:esp32:nodemcu-32s
#PORT := /dev/ttyUSB0
PORT := COM3

SOURCES := $(wildcard *.ino)

all: build upload

build: $(SOURCES)
	$(CC) compile -b $(BOARD) -e

upload:
	$(CC) upload --input-dir build/$(subst :,.,$(BOARD))/ -b $(BOARD) -p $(PORT)

clean:
	-rm -rf build
