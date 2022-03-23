#!/bin/sh
#set G102 audio throughway begin
#dafult headphone out,default headphone volume is 40
echo "set G102 audio pass through"
amixer cset iface=MIXER,name='headphone volume' 40
amixer cset iface=MIXER,name='HP_L Mux' 1
amixer cset iface=MIXER,name='HP_R Mux' 1
amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
amixer cset iface=MIXER,name='Headphone Switch' 1
tinyplayer $1 $2
