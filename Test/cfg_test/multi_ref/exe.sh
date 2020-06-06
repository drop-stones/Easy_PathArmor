#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_GEN=$PATH_ARMOR/cfg_generation
CFG_TEST=$PATH_ARMOR/Test/cfg_test

NAME=multi_ref
WD=$CFG_TEST/$NAME
ENTRY=0x400557
EXIT=0x40056f

echo -e "$ cat $NAME.cpp\n"
cat $WD/$NAME.cpp
echo -e "\n$ $CFG_GEN/a.out $NAME config.map $ENTRY $EXIT cfg.txt\n"
$CFG_GEN/a.out $WD/$NAME $WD/config.map $ENTRY $EXIT $WD/cfg.txt
echo -e "$ cat cfg.txt\n"
cat $WD/cfg.txt
echo -e "\n\n$ $CFG_TEST/cfg_load cfg.txt"
$CFG_TEST/cfg_load $WD/cfg.txt
