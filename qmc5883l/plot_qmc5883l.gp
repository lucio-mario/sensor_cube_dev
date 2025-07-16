# Initial Configuration
set terminal wxt enhanced size 1200,900 title 'Visualization of QMC5883L Data (Real Time)'
DATA_FILE = "live_data.txt"
set grid
set datafile separator ","

set multiplot layout 2,1 title "QMC5883L Data in Real Time" font ",14"

# Graph 1: Magnetic Field
set key top left
set title "Magnetic Field"
set ylabel "Raw Sensor Value"
plot DATA_FILE using 1 with lines title 'MagX', \
     '' using 2 with lines title 'MagY', \
     '' using 3 with lines title 'MagZ'

# Graph 2: Compass (Direction)
set key top left
set title "Compass (True North)"
set xlabel "Time (Samples)"
set ylabel "Direction (° x10)"
plot DATA_FILE using 4 with lines title 'Direction (Heading)' lc "blue"

unset multiplot

while (1) {
    pause 0.1
    replot
}
