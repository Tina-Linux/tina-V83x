#!/bin/sh
tinatest=true

if [ "x$tinatest" = "xtrue" ]; then
    keypath="/base/audio/common/captureloop"

    volume=$(mjson_fetch "${keypath}/volume")


    card_array=$(mjson_fetch "${keypath}/card")
    sr_array=$(mjson_fetch "${keypath}/sample_rate")
    sb_array=$(mjson_fetch "${keypath}/sample_bits")
    ch_array=$(mjson_fetch "${keypath}/channels")
    music_sec=$(mjson_fetch "${keypath}/music_sec")
    if [ $(mjson_fetch "${keypath}/function") = 0 ]; then
        is_playback=true
        wav_dir=$(mjson_fetch "${keypath}/wav_dir")
    else
        is_capture=true
        record_wav_dir=$(mjson_fetch "${keypath}/record_wav_dir")
    fi
    echo wav_dir:${wav_dir}
    echo record_wav_dir:${record_wav_dir}
    external_device=$(mjson_fetch "${keypath}/external_device")
    loop_all=true
fi


:<<!
    Playback Test

    args:
    {
        card:               string
        sample rate:        array[]
        sample resolution:  array[]
        channels:           array[]
        wav pre-path:       string
        device require:     string
        playback pcm device:string
        capture pcm device: string
    }

Args For Test:
card="audiocodec,0 audiocodec,1"
sr_array="16000 48000"
sb_array="16 24"
ch_array="2"

music file: rate-bits-ch-sec.wav

!

#default_para="playback"
#default_para="capture"
[ "x$default_para" = "xplayback" ] && {
    card_array="audiocodec,0"
    #sr_array="8000 11025 16000 22050 24000 32000 44100 48000 88200 96000 176400 192000"
    sr_array="8000 11025 16000"
    #sr_array="22050 24000 32000"
    #sr_array="44100 48000 88200"
    #sr_array="96000 176400 192000"
    #sr_array="44100 48000 88200 96000 176400"
    sb_array="S16_LE"
    ch_array="1 2"
    music_sec=10

    is_playback=true
    is_capture=0

    loop_all=true
    #loop_all=

    #wav_dir=/mnt/UDISK/test_wav/common
    wav_dir=/tmp/
    record_wav_dir=/tmp/record_files
    #pcm_device="PlaybackSpeaker"
    require_device="Speaker"
}
[ "x$default_para" = "xcapture" ] && {
    card_array="audiocodec,0"
    #sr_array="8000 11025 16000 22050 24000 32000 44100 48000 88200 96000 176400 192000"
    #sr_array="8000"
    #sr_array="44100 48000 88200 96000 176400"
    sr_array="8000 11025 16000"
    #sr_array="22050 24000 32000"
    #sr_array="44100 48000 88200"
    #sr_array="96000 176400 192000"
    sb_array="S16_LE"
    ch_array="1 2 3"
    music_sec=3

    is_playback=0
    is_capture=true

    loop_all=true
    #loop_all=

    #wav_dir=/mnt/UDISK/test_wav/common
    wav_dir=/tmp
    record_wav_dri=/tmp/record_files
    #pcm_device="PlaybackSpeaker"
}


pr_info() {
    if [ "x$tinatest" = "xtrue" ]; then
        ttips "$*"
    else
        printf "$*\n"
    fi
}
get_confirm() {
    if [ "x$tinatest" = "xtrue" ]; then
        ttrue "$*"
        if [ $? -eq "0" ]; then
            return 1
        else
            return 0
        fi
    else
        local input=0
        printf "$*"
        read input
        if [ "x$input" = "xN" -o "x$input" = "xn" ]; then
            return 0
        elif [ "x$input" = "xY" -o "x$input" = "xy" ]; then
            return 1
        else
            return 1
        fi
    fi
}


loop_func() {
    local fail_count=0
    # Start traverse Sound Card
    for card in $card_array; do
        # Start traverse Sample rate
        for sr in $sr_array; do
            # Start traverse Sample resolution(sample bits)
            for sb in $sb_array; do
                sb=`echo $sb |tr '[A-Z]' '[a-z]'`
                # Start traverse channels
                for ch in $ch_array; do
                    ch_name="$ch"
                    if [ $ch = 1 ]; then
                        ch_name="mono"
                    fi
                    if [ $ch = 2 ]; then
                        ch_name="stereo"
                    fi
                    wav_file=$sr-$ch_name-$sb-${music_sec}s.wav
                    ttips   "Testing [$card] with args:" -n \
                            "sample rate: [$sr]" -n \
                            "sample resolution: [$sb]" -n \
                            "channels: [$ch]"
                    if [ $pcm_device ]; then
                            device=$pcm_device
                    else
                            device=hw:$card
                    fi
                    # capture process
                    if [ "x$is_capture" = "xtrue" ]; then
                        [ -d $record_wav_dir ] || {
                            mkdir -p $record_wav_dir
                        }
                        wav_path=$record_wav_dir/record_$wav_file
                        cmd="arecord -D"$device" -t wav "$wav_path" -f $sb -r $sr -c $ch -d $music_sec -q"
                        #echo "command:$cmd";

                        alsaucm -c $card set _verb Record set _enadev $external_device
                        $cmd
                        case $? in
                            0)
                                result=${result}" -n $wav_file : support"
                                get_confirm "Capture music[$wav_path] finish,Please confirm this file!"
                                if [ $? == 0 ]; then
                                    result=${result}", but can not hear the music!"
                                    if [ "x$loop_all" != "xtrue" ]; then
                                        let fail_count++
                                    fi
                                fi
                                ;;
                            *)
                                result=${result}" -n $wav_file : not support"
                                ;;
                        esac
                        alsaucm -c $card set _verb Record set _enadev $external_device set _disdev $external_device
                    fi

                    # playback process
                    if [ "x$is_playback" = "xtrue" ]; then
                        wav_path=$wav_dir/$wav_file
                        [ ! -f $wav_path ] && {
                            pr_info "wav file isn't exist.[$wav_path]"
                            result=${result}" -n $wav_file : not exist"
                            continue
                        }
                        cmd="aplay -D"$device"  "$wav_path" -q"
                        #echo "command:$cmd";
                        alsaucm -c $card set _verb Play set _enadev $external_device
                        control=`alsaucm -c $card get PlaybackVolume/$external_device/Play | cut -d "=" -f 2-`
                        control=`echo ${control%" "*}`
                        amixer -D"$device" cset "$control" $volume -q
                        $cmd
                        case $? in
                            0)
                                result=${result}" -n $wav_file : support"
                                get_confirm "Can you hear the music?"
                                if [ $? == 0 ]; then
                                    result=${result}", but can not hear the music!"
                                    let fail_count++
                                fi
                                ;;
                            *)
                                result=${result}" -n $wav_file : not support"
                                ;;
                        esac
                        alsaucm -c $card set _verb Play set _enadev $external_device set _disdev $external_device
                    fi
                    [ "x$loop_all" != "xtrue" ] && [ $fail_count != 0 ] && {
                        return 0
                    }
                done # End traverse channels
            done # End traverse Sample resolution
        done # End traverse Sample rate
    done # End traverse Sound Card
}

result="Test Result:"
loop_func
get_confirm ${result}"  -n Is the result right?"
[ $? != 0 ] && exit 0
exit 1
