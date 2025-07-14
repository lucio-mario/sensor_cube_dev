# Initial Configuration
set terminal wxt enhanced size 900,900 title 'GPS Visualization (Real Time)'
DATA_FILE = "gps_data.txt"
set datafile separator ","
set grid
set pointsize 1.5

# --- DEFINIÇÃO DOS PLOTS ---
set multiplot layout 2,1 title "GPS Data in Real Time" font ",14"

# Graph 1: Localization (Track Log)
# Filters line that contains 'GGA' (whether $GNGGA or $GPGGA)
# Column 5: Longitude, Column 3: Latitude
set title "Localization (Latitude vs. Longitude)"
set xlabel "Longitude"
set ylabel "Latitude"
plot sprintf("< grep 'GGA' %s", DATA_FILE) using 5:3 with points title 'Position'

# Graph 2: Signal Quality
# Column 8 of the sentence GGA is the number of sattelites being tracked
set title "Quality of GPS Signal"
set xlabel "Time (Samples)"
set ylabel "Number of Sattelites"
plot sprintf("< grep 'GGA' %s", DATA_FILE) using 8 with lines title 'Visible Sattelites' lc "green"

unset multiplot

while(1) {
    pause 1
    replot
}
