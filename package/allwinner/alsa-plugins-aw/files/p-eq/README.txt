##### how to use awequal #####

1. Prepare the configuration file "awequal.conf"

2. Define a ALSA pcm device in ALSA configuration file, such as /etc/asound.conf.
   The pcm device can be as follows:

pcm.eq {
    type awequal
    slave.pcm "hw:0,0"      # replace this with your appropriate pcm device
    config_file "/etc/awequal.conf"     # the path of the configuration file
}

3. Finally, you can use the following command to run the awequal plugin:

aplay -D eq yourmusic.wav
