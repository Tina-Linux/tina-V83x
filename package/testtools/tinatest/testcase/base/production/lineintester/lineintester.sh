#!/bin/sh

target=$(get_target)
path_in_config_tree="/base/production/lineintester"
headphone_volume=$(mjson_fetch ${path_in_config_tree}/headphone_volume)
duration_sec=$(mjson_fetch ${path_in_config_tree}/duration_sec)

case "$target" in
    r18-*)
        amixer cset name='headphone volume' $headphone_volume

        amixer cset iface=MIXER,name='Left Output Mixer LINEINL Switch' 1
        amixer cset iface=MIXER,name='Right Output Mixer LINEINR Switch' 1
        amixer cset iface=MIXER,name='HP_L Mux' 'Left Analog Mixer HPL Switch'
        amixer cset iface=MIXER,name='HP_R Mux' 'Right Analog Mixer HPR Switch'
        amixer cset iface=MIXER,name='Headphone Switch' 1

        sleep $duration_sec

        amixer cset iface=MIXER,name='Headphone Switch' 0
        amixer cset iface=MIXER,name='Left Output Mixer LINEINL Switch' 0
        amixer cset iface=MIXER,name='Right Output Mixer LINEINR Switch' 0
        ;;
    *)
        echo "Not support this platform."
        exit 1
        ;;
esac
