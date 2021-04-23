#!/bin/bash

# Exit if something fails
set -e

cd build
sudo make uninstall

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    kquitapp5 krunner
fi

# If KRunner does not get started using the shortcut we have to autostart it
krunner_version=$(krunner --version | cut -d " " -f2)
major_version=$(echo $krunner_version | cut -d "." -f -1)
minor_version=$(echo $krunner_version | cut -d "." -f2)
if [[ (("$major_version" < "5")) || (("$minor_version" < "17")) ]]
then
    kstart5 --windowclass krunner
fi
