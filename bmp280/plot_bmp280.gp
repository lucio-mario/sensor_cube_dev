set terminal wxt enhanced size 1200,600 title 'Visualization of BMP280 Barometer (Real Time)'
DATA_FILE = "bmp280_data.txt"
set grid
set datafile separator ","

set multiplot layout 2,1 title "BMP280 Data in Real Time" font ",14"

# Graph 1: Temperature
set key top left
set title "Barometer Temperature"
set ylabel "Degrees Celsius (°C)"
plot DATA_FILE using ($1 / 100.0) with lines title 'BMP280 Temperature' lc "red"

# Graph 2: Atmospheric Pressure
set key top left
set title "Atmospheric Pressure"
set xlabel "Time (Samples)"
set ylabel "Pressure (hPa)"
plot DATA_FILE using ($2 / 100.0) with lines title 'Pressão' lc "purple"

unset multiplot

while(1) {
    pause 0.1
    replot
}
