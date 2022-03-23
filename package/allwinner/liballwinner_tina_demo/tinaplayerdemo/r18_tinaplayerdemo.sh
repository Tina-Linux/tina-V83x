#!/bin/sh
#set r18 audio throughway begin
#dafult headphone out,default headphone volume is 30
echo "set r18 audio pass through"
amixer cset name='AIF1IN0R Mux' 'AIF1_DA0R'
amixer cset name='AIF1IN0L Mux' 'AIF1_DA0L'
amixer cset name='DACR Mixer AIF1DA0R Switch' 1
amixer cset name='DACL Mixer AIF1DA0L Switch' 1
amixer cset name='DAC volume' 160
amixer cset name='HP_R Mux' 'DACR HPR Switch'
amixer cset name='HP_L Mux' 'DACL HPL Switch'
amixer cset name='Headphone Switch' 1
amixer cset name='External Speaker Switch'  0
#set r18 audio throughway finish
realtinaplayerdemo
