
filename="images/matrix-animation.gif"
gc=0.200
width=10
skip=5

set terminal gif notransparent animate delay 0.5 loop 0 optimize size 800,600

set output filename
set xlabel "Neuron Positions"
set ylabel "Neuron Positions"
set cbrange [-80:20]
set xtics 1
set ytics 1
set xrange [-0.5:width-0.5]
set yrange [-0.5:width-0.5]

frames=1000
t=0.0
skip=5
do for [i=0.:frames-1] {
t = 0.01 * i * skip
print sprintf("%2.2f%% done @ frame %1.0f of %1.0f @ t=%1.3fms for ",100.0*i/frames,i,frames,t).filename
set title sprintf("Neuron Mesh (coupling g_c=%1.4f) \nat t=%1.3fms; frame #%4.0f;   ",gc,t,i).filename
plot "output/0.dat" index i*skip matrix with image
}

unset output
print "Render complete.\n"