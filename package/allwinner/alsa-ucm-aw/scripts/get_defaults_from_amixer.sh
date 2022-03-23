#!/bin/sh

# Only tested for controls of BOOLEAN, INTEGER, ENUMERATED, not sure whether
# it works well or not for other types

help_msg="
This script is will transform the current amixer contents to SectionDefaults in
UCM configurations.

USAGE:
    save_defaults_from_amixer.sh <saved_file>

NOTE:
    This script use sh to parse but not bash, and sh on Ubuntu is linked to dash
    by default, which may cause unexpected results. Therefore, you'd better test
    this script on your target machine first to ensure that it works well.
"

if [ "x$1" == "x-h" ]; then
    echo "$help_msg"
    exit 0
fi

results=

contents=$(amixer contents)
controls=$(amixer controls)

old_IFS=$IFS
IFS=$'\n'
for line in $controls; do
    control_msg=$(echo "$contents" | grep -A 2 "$line")
    temp=${control_msg##*; type=}
    type=${temp%%,*}
    case $type in
        ENUMERATED)
            msg=$(echo "$control_msg" | grep ",items=")
            items=${msg##*,items=}
            trailing_lines=$(expr $items + 2)
            control_msg=$(echo "$contents" | grep -A $trailing_lines "$line")
            value_num=${control_msg##*: values=}
            value_item=$(echo "$control_msg" | grep "; Item #$value_num")
            value=${value_item##*; Item #? }
            ;;
        *)
            value=${control_msg##*: values=}
            ;;
    esac
    results="$results\ncset \"$line $value\""
done
IFS=$old_IFS

echo -e "$results"
