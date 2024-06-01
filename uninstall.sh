#!/usr/bin/env bash

set -e
cd build
sudo make uninstall

# KRunner needs to be restarted for the changes to be applied
krunner_version=$(krunner --version | grep -oP "(?<=krunner )\d+")
if pgrep -x krunner > /dev/null
then
    if [[ "$krunner_version" == "6" ]]; then
        kquitapp6 krunner
    else
        kquitapp5 krunner
    fi
fi

