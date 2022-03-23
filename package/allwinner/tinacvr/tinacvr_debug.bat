set NAME=tinacvr

::adb push \tina\package\allwinner\tinacvr\res\minigui\MiniGUI.cfg /usr/local/etc
::adb push \tina\package\allwinner\tinacvr\res\minigui\sunxi-keyboard.kl /etc
::adb shell chmod 777 /usr/local/etc/MiniGUI.cfg
::adb shell chmod 777 etc/sunxi-keyboard.kl

adb push \git\tina\out\r11-GW\compile_dir\target\tinacvr\src\tinacvr /usr/bin/%NAME%
adb push \git\tina\out\r11-GW\compile_dir\target\tinacvr\res\cfg /usr/res/cfg
adb push \git\tina\out\r11-GW\compile_dir\target\tinacvr\res\data /usr/res/data
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\font /usr/res/font
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\lang /usr/res/lang
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\menu /usr/res/menu
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\others /usr/res/others
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\topbar /usr/res/topbar
::adb push \tina\out\r11-GW\compile_dir\target\tinacvr\res\sound /usr/res/sound

adb shell chmod 777 /usr/bin/%NAME%
adb shell chmod 777 /usr/res/cfg/*
adb shell chmod 777 /usr/res/data/*
::adb shell chmod 777 /usr/res/font/*
::adb shell chmod 777 /usr/res/lang/*
::adb shell chmod 777 /usr/res/menu/*
::adb shell chmod 777 /usr/res/others/*
::adb shell chmod 777 /usr/res/topbar/*
::adb shell chmod 777 /usr/res/sound/*

::adb shell %NAME%

pause
