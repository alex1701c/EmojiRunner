#!/usr/bin/env bash

set -e
cd build
sudo make uninstall

# KRunner needs to be restarted for the changes to be applied
krunner_version=$(krunner --version | grep -oP "(?<=krunner )\d+")
if pgrep -x krunner > /dev/null
then
    kquitapp"$krunner_version" krunner
fi
