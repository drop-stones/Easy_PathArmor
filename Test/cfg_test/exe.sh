#!/bin/bash

## ./cfg_test ../test01/test01 configure.map 0x400749 0x400759

LINK_PATH="/home/linuxbrew/.linuxbrew/Cellar/boost/1.72.0_2/lib"

LD_LIBRARY_PATH=${LINK_PATH} ./cfg_test ../test01/test01 configure.map 0x400746 0x40075e
echo -e "\n\n start cfg_load\n"
LD_LIBRARY_PATH=${LINK_PATH} ./cfg_load
