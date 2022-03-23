set NAME=r11-board

adb push \workspace\F35\tina\out\r11-R11_pref1\compile_dir\target\r11-board\src\r11-board /tmp/%NAME%

::adb push \R11\tina\package\minigui\r11-board\src\res\menu\video.png /tmp/
::adb push \R11\tina\package\minigui\r11-board\src\res\menu\sensor.png /tmp/

adb shell chmod 777 /tmp/%NAME%
::adb shell chmod 777 /tmp/video.png
::adb shell chmod 777 /tmp/sensor.png

::adb shell %NAME%

pause
