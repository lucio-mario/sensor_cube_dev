HI_OFFSET_X = 512.0000
HI_OFFSET_Y = -10.0000
HI_OFFSET_Z = 1237.5000

SI_SCALE_X = 1.0092
SI_SCALE_Y = 0.7260
SI_SCALE_Z = 1.5829

set terminal wxt enhanced size 1000,800 title 'Magnetometer Calibration'
set title "Magnetometer Data: Raw vs. Corrected" font ",14"

set view equal xyz
set grid
set xlabel "MagX" font ",12"
set ylabel "MagY" font ",12"
set zlabel "MagZ" font ",12"
set ticslevel 0

DATA_FILE = "live_data.txt"
set datafile separator ","

correctX(col_x) = (col_x - HI_OFFSET_X) * SI_SCALE_X
correctY(col_y) = (col_y - HI_OFFSET_Y) * SI_SCALE_Y
correctZ(col_z) = (col_z - HI_OFFSET_Z) * SI_SCALE_Z

splot DATA_FILE using 1:2:3 with points pt 7 ps 0.5 lc "red" title "Raw Data", \
      '' using (correctX($1)):(correctY($2)):(correctZ($3)) with points pt 7 ps 0.5 lc "green" title "Corrected Data"
