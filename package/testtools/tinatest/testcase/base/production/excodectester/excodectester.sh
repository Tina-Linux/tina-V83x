#!/bin/sh

# Record via the microphone and then play the record through the headphone.

target=`get_target`
path_in_config_tree="/base/production/excodectester"
record_duration_sec=`mjson_fetch $path_in_config_tree/record_duration_sec`
record_audio_path=`mjson_fetch $path_in_config_tree/record_audio_path`
headphone_volume=`mjson_fetch $path_in_config_tree/headphone_volume`
audio_file=`mjson_fetch $path_in_config_tree/audio_file`

case $target in
    r40-*)
        # record
        amixer cset iface=MIXER,name='Left Input Mixer MIC1 Boost Switch' 1
        amixer cset iface=MIXER,name='Right Input Mixer MIC1 Boost Switch' 1
        # play
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='Headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
        amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
        amixer cset iface=MIXER,name='HPL Mux' 1
        amixer cset iface=MIXER,name='HPR Mux' 1
    ;;
    r16-*)
        # record
        amixer cset iface=MIXER,name='ADC input gain' 3
        amixer cset iface=MIXER,name='ADC volume' 180
        amixer cset iface=MIXER,name='MIC1 boost amplifier gain' 5
        amixer cset iface=MIXER,name='AIF1 AD0L Mixer ADCL Switch' 1
        amixer cset iface=MIXER,name='AIF1 AD0R Mixer ADCR Switch' 1
        amixer cset iface=MIXER,name='LEFT ADC input Mixer MIC1 boost Switch' 1
        amixer cset iface=MIXER,name='RIGHT ADC input Mixer MIC1 boost Switch' 1
        # play
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
        amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
        amixer cset iface=MIXER,name='HP_L Mux' 0
        amixer cset iface=MIXER,name='HP_R Mux' 0
    ;;
    r6-*)
        #record
        #play
        amixer cset iface=MIXER,name='head phone volume' $headphone_volume
    ;;
    *)
        echo "This test case does not support current platform."
        exit 1
    ;;
esac

mkdir -p `dirname $record_audio_path`
arecord -D "hw:1,0" -d $record_duration_sec -f "S16_LE" -c 2 -r 16000 $record_audio_path -v | aplay $audio_file -D "hw:0,0" -d $record_duration_sec -v
if [ $? -ne "0" ]; then
    echo "Record failed!"
    exit 1
fi

aplay $record_audio_path -D "hw:0,0" -v
