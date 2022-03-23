#!/bin/sh
#set c600 audio throughway begin
#dafult headphone out,default headphone volume is 30
echo "set c600 audio pass through"
amixer cset iface=MIXER,name='head phone volume' 30
#set c600 audio throughway finish
tinyplayer $1 $2
