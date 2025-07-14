#!/bin/bash

# Variables
SERIAL_PORT="/dev/ttyACM0"
GPS_FILE="gps_data.txt"
TMP_FILE="${GPS_FILE}.tmp"
BAUD_RATE=9600
MAX_LINES=200

# Cleans old file
> "$GPS_FILE"

echo "Configuring the serial port: $SERIAL_PORT a $BAUD_RATE baud..."
stty -F "$SERIAL_PORT" $BAUD_RATE raw -echo

echo "Starting GPS logger"
echo "GPS Data -> $GPS_FILE"

# Redirects the output of the serial port to a for loop
cat "$SERIAL_PORT" | while IFS= read -r line; do
    echo "$line" >> "$GPS_FILE"
    tail -n "$MAX_LINES" "$GPS_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$GPS_FILE"
done
