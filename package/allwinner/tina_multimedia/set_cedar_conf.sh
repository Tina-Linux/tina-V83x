#!/bin/bash
line=`sed -n "/\<$1\>/p" $3`
if [ -n "$line" ]
then
if [ $4 == "y" ]
then
sed -i "s/$line/$1 = $2/g" $3
fi
else
if [ $4 == "y" ]
then
sed -i '3a '$1' = '$2'' $3
fi
fi
