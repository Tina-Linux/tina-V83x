#!/bin/sh

keypath="/base/audio/mic_capture"

capture_pcm_device=$(mjson_fetch "${keypath}/capture_pcm_device")
capture_channels=$(mjson_fetch "${keypath}/capture_channels")
capture_format=$(mjson_fetch "${keypath}/capture_format")
capture_rate=$(mjson_fetch "${keypath}/capture_rate")
capture_duration_sec=$(mjson_fetch "${keypath}/capture_duration_sec")
record_file=$(mjson_fetch "${keypath}/record_file")
remove_record_file=$(mjson_fetch "${keypath}/remove_record_file")
playback_pcm_device=$(mjson_fetch "${keypath}/playback_pcm_device")

[ ! -d "$(dirname ${record_file})" ] && mkdir -p "$(dirname ${record_file})"

ttips "Start recording (in $capture_duration_sec seconds)"
arecord -D "$capture_pcm_device" -c "$capture_channels" -f "$capture_format" \
    -r "$capture_rate" -d "$capture_duration_sec" "$record_file"
if [ $? -ne "0" ]; then
    ttips "Error occurred when recording"
    exit 1
fi

ttips "Finish recording, and start playing the record"
aplay -D "$playback_pcm_device" "$record_file"
if [ $? -ne "0" ]; then
    ttips "Error occurred when playing"
    exit 1
fi

ttrue "Finish playing the record. Can you hear the sound?"
exit_code=$?

[ "x$remove_record_file" == "xtrue" ] && rm -f "$record_file"

exit $exit_code
