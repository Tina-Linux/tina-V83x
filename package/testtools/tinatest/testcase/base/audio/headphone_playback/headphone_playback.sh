#!/bin/sh

keypath="/base/audio/headphone_playback"

pcm_device=$(mjson_fetch "${keypath}/pcm_device")
wav_file=$(mjson_fetch "${keypath}/wav_file")

if [ ! -f "$wav_file" ]; then
    ttips "No such file: \"$wav_file\""
    exit 1
fi

ttips "Starting playing with headphone"
aplay -D "$pcm_device" "$wav_file"
if [ $? -ne "0" ]; then
    ttips "Error occurred when playing"
    exit 1
fi
ttrue "Finish playing. Can you hear the sound from headphone?"
exit $?
