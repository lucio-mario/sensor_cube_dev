# Variables
SERIAL_PORT="/dev/ttyACM0"
DATA_FILE="bmp280_data.txt"
TMP_FILE="${DATA_FILE}.tmp"
BAUD_RATE=9600

> "$DATA_FILE"

echo "Configuring the serial port: $SERIAL_PORT a $BAUD_RATE baud..."
stty -F "$SERIAL_PORT" $BAUD_RATE raw -echo

echo "Starting BMP280 logger"
echo "BMP280 Data -> $DATA_FILE"

while IFS= read -r linha;
do
  linha_limpa=$(echo "$linha" | tr -d '\r')
  echo "$linha_limpa" >> "$DATA_FILE"
  tail -n 500 "$DATA_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$DATA_FILE"
done < "$SERIAL_PORT"
