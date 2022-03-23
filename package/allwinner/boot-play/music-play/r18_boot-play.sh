#!/bin/sh
#set r18 audio throughway begin
#dafult headphone out,default headphone volume is 30
echo "set r18 audio pass through"
amixer cset iface=MIXER,name='headphone volume' 30
amixer cset iface=MIXER,name='AIF1OUT0R Mux' 1
amixer cset iface=MIXER,name='AIF1OUT0L Mux' 1
amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
amixer cset iface=MIXER,name='HP_L Mux' 0
amixer cset iface=MIXER,name='HP_R Mux' 0
amixer cset iface=MIXER,name='Headphone Switch' 1
#set r18 audio throughway finish
tinyplayer $1 $2
