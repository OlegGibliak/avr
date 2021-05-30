PROJECT_NAME := nrf_test
export C_INCLUDE_PATH = inc

MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) )

PORT       := /dev/ttyUSB1

DEVICE     := atmega328p
CLOCK      := 16000000
PROGRAMMER := -c arduino -P$(PORT)	 -D
SRC_DIR	   := src/
INC_DIR    := C_INCLUDE_PATH
OBJ_DIR    := build/
SIZE       := avr-size --format=avr --mcu=$(DEVICE)
OBJCOPY    := avr-objcopy -j .text -j .data
#Команда запуска avrdude. Ее нужно скопировать из Arduino IDE.
#AVRDUDE = avrdude -C/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/etc/avrdude.conf -carduino -P/dev/tty.usbserial-A600dAAQ -b19200 -D -p atmega168
BAUDRATE = 115200
AVRDUDE = avrdude -F -v -patmega328p $(PROGRAMMER) -b $(BAUDRATE)
#  -v -patmega328p -carduino -P/dev/ttyUSB0 -b57600 -D -Uflash:w:/tmp/arduino_build_223875/Blink.ino.hex:i 
MK := mkdir
RM := rm -rf
NO_ECHO := @

ROOT_DIR := ../..
#source common to all targets
C_SOURCE_FILES += \
$(abspath $(ROOT_DIR)/avr_drivers/serial/serial.c) \
$(abspath $(ROOT_DIR)/avr_drivers/spi/spi.c) \
$(abspath $(ROOT_DIR)/components/logger/logger.c) \
$(abspath $(ROOT_DIR)/components/timer_timestamp/src/timer_timestamp.c) \
$(abspath $(ROOT_DIR)/components/app_timer/src/app_timer.c) \
$(abspath $(ROOT_DIR)/radio/nrf2401/nrf2401.c) \
$(abspath $(ROOT_DIR)/radio/nrf2401/radio.c) \
$(abspath ./main.c)\


#includes common to all targets
INC_PATHS  = -I$(C_INCLUDE_PATH)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/avr_drivers/serial)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/avr_drivers/spi)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/components/logger)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/components/common)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/components/timer_timestamp/inc)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/components/app_timer/inc)
INC_PATHS += -I$(abspath ./$(ROOT_DIR)/radio/nrf2401)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

CFLAGS  = -DARDUINO_BOARD
CFLAGS += -Wall -Werror -O3 -g3
CFLAGS += -DF_CPU=$(CLOCK)

# Modules enable
CFLAGS += -DMODULE_LED_DBG
CFLAGS += -DCONFIG_ASSERT_ENABLE

LDFLAGS += -Wl,-Map,$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).map

COMPILE = avr-gcc -mmcu=$(DEVICE) -std=c99

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(dir $(C_SOURCE_FILES) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

vpath %.c $(C_PATHS)

OBJECTS = $(C_OBJECTS)

all: clean
	$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e main

OUTPUT_FILENAME := $(PROJECT_NAME)

main: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(COMPILE) $(LDFLAGS) $(INC_PATHS) $(OBJECTS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	@echo $(MAKEFILE_NAME)
	$(MK) $@

#Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(COMPILE) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex:
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o

clean:
	$(RM) $(BUILD_DIRECTORIES)

flash: all
	$(AVRDUDE) -U flash:w:$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex:i

terminal: flash
	picocom -b 9600 $(PORT)
