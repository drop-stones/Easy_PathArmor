#!/bin/bash

cd ~/pin/pin-3.6-97554-g31f0a167d-gcc-linux
./pin -t ~/code/PathArmor/Pin/obj-intel64/track.so -- \
    ~/code/PathArmor/Test/test01/test01 0
