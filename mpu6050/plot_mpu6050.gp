# Initial Configuration
set terminal wxt enhanced size 1200,900 title 'Visualization of MPU-6050 Data (Real Time)'
DATA_FILE = "live_data.txt"
set grid
set datafile separator ","

set multiplot layout 3,1 title "MPU-6050 Data in Real Time" font ",14"

# Graph 1: Linear Acceleration
set key top left
set title "Linear Acceleration"
set ylabel "Force (g)"
plot DATA_FILE using 1 with lines title 'AcX', \
     '' using 2 with lines title 'AcY', \
     '' using 3 with lines title 'AcZ'

# Graph 2: Angular Velocity
set key top left
set title "Angular Velocity"
set ylabel "Degrees / Second (°/s)"
plot DATA_FILE using 5 with lines title 'GyX', \
     '' using 6 with lines title 'GyY', \
     '' using 7 with lines title 'GyZ'

# Graph 3: Temperature
set key top left
set title "Temperature"
set xlabel "Time (Samples)"
set ylabel "Degrees Celsius(°C)"
plot DATA_FILE using 4 with lines title 'Temperatura' lc "red"

unset multiplot

while(1) {
    pause 0.01

    replot
}
