set encoding utf8
set grid xtics ytics ls 0 lt 1
show grid
set xlabel 'Расстояние, бит'
set ylabel 'R'
set xrange [0 to 32]
plot 'periodic.data' using 1:(abs($2)) title "Автокорреляционная функция" with boxes fs solid 0.8
pause -1
