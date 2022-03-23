#!/bin/sh
#set r40 audio throughway begin
#dafult headphone out,default headphone volume is 40
echo "set r40 audio pass through"
amixer cset iface=MIXER,name='Headphone volume' 40
amixer cset iface=MIXER,name='HPL Mux' 1
amixer cset iface=MIXER,name='HPR Mux' 1
amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
amixer cset iface=MIXER,name='Headphone Switch' 1
#set r40 audio throughway finish
tinyplayer $1 $2
