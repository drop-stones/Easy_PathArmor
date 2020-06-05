#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_GEN=$PATH_ARMOR/cfg_generation
CFG_TEST=$PATH_ARMOR/Test/cfg_test
WD=$CFG_TEST/bb_divide

ENTRY=0x400546
EXIT=0x400570

echo -e "$ cat bb_divide.cpp\n"
cat $WD/bb_divide.cpp
echo -e "\n$ $CFG_GEN/a.out bb_divide config.map $ENTRY $EXIT cfg.txt\n"
$CFG_GEN/a.out $WD/bb_divide $WD/config.map $ENTRY $EXIT $WD/cfg.txt
echo -e "$ cat cfg.txt\n"
cat $WD/cfg.txt
echo -e "\n\n$ $CFG_TEST/cfg_load cfg.txt"
$CFG_TEST/cfg_load $WD/cfg.txt
