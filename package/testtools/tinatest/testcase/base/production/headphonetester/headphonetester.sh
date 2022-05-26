#!/bin/sh

# Play an audio file through the headphone.

target=`get_target`
path_in_config_tree="/base/production/headphonetester"
headphone_volume=`mjson_fetch $path_in_config_tree/headphone_volume`
audio_file=`mjson_fetch $path_in_config_tree/audio_file`

case "$target" in
    r40-*)
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='Headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
        amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
        amixer cset iface=MIXER,name='HPL Mux' 1
        amixer cset iface=MIXER,name='HPR Mux' 1
    ;;
    r16-*)
        amixer cset iface=MIXER,name='Headphone Switch' 1
        amixer cset iface=MIXER,name='headphone volume' $headphone_volume
        amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
        amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
        amixer cset iface=MIXER,name='HP_L Mux' 0
        amixer cset iface=MIXER,name='HP_R Mux' 0
    ;;
    r6-*)
        echo "platform: r6"
        amixer cset iface=MIXER,name='head phone volume' $headphone_volume
    ;;
    *)
        echo "This test case does not support current platform."
        exit 1
    ;;
esac
echo "start playing audio..."
aplay $audio_file -v
