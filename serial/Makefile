# Directories
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
SRC_DIR = src

# ----------------------------------------------------------------------
# Files
TARGET_NAME = serial
TARGET = $(BIN_DIR)/$(TARGET_NAME)
MAIN_FILE = $(SRC_DIR)/main.c
SOURCES = $(shell find src -name '*.c')
SOURCES_NAMES = $(patsubst $(SRC_DIR)/%,%,$(SOURCES))
HEADERS = $(shell find src -name '*.h')
OBJECTS = $(patsubst %.c, %.o, $(OBJ_DIR)/$(SOURCES_NAMES))
OBJECT_NAMES = $(patsubst $(OBJ_DIR)/%,%,$(OBJECTS))

# ----------------------------------------------------------------------
# Toolchain
CC = avr-gcc
AVRDUDE = avrdude
OBJCOPY = avr-objcopy
RM = rm
FORMAT = # Code formatter
SERIAL_MONITOR = picocom

# ----------------------------------------------------------------------
#  Don't know
CSTANDARD = -std=c99
F_CPU = 16000000
MCU = atmega328p
AVRDUDE_PROGRAMMER = arduino
MCU_PORT = /dev/ttyACM0
AVDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_BAUD_RATE = 115200
SERIAL_MONITOR_BAUD_RATE = 9600

# ----------------------------------------------------------------------
# Flags
WFLAGS = -Wall -Wextra -Werror -Wshadow
CFLAGS = $(WFLAGS) -Os $(CSTANDARD) -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL
LFLAGS = # Linking Flags
AVRDUDE_FLAGS = -F -V -p $(MCU) -P $(MCU_PORT) -c $(AVRDUDE_PROGRAMMER) -b $(AVRDUDE_BAUD_RATE) $(AVDUDE_WRITE_FLASH)
OBJCOPY_FLAGS = -O ihex -R .eeprom -j .text -j .data

# ----------------------------------------------------------------------
# Phonies
.PHONY: all clean build bin hex flash run print

## Buids, flashes and opens serial monitor
all: build flash run

## Removes build directory
clean:
	@$(RM) -rf $(BUILD_DIR)

## Buids target to flash
build: bin hex

# Builds binary
bin: $(TARGET).bin

## Converts .bin to .hex to flash
hex: $(TARGET).hex

## Flashes
flash: $(TARGET).hex
	sudo $(AVRDUDE) $(AVRDUDE_FLAGS)

## Opens serial monitor
run:
	sudo $(SERIAL_MONITOR) -b $(SERIAL_MONITOR_BAUD_RATE) $(MCU_PORT)

## Debug
print:
	@echo SOURCES = $(SOURCES)
	@echo SOURCES_NAMES = $(SOURCES_NAMES)
	@echo HEADERS = $(HEADERS)
	@echo OBJECTS = $(OBJECTS)
	@echo OBJECT_NAMES = $(OBJECT_NAMES)

# ----------------------------------------------------------------------
# Compilation and linking
$(TARGET).hex: $(TARGET).bin
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(TARGET).bin $(TARGET).hex

$(TARGET).bin: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
