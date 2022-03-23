#!/bin/ash

fbtest

ttrue "Can you see the screen color change?"
if [ $? = 0 ]
then
echo "GOOD, screen color change success."
return 0
else
echo "ERROR, screen color change failed."
return 1
fi
