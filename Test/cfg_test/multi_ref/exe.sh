#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_PATH=$PATH_ARMOR/cfg_generation
CFG_TEST_PATH=$PATH_ARMOR/Test/cfg_test
PWD=$CFG_TEST_PATH/multi_ref

echo -e "$ cat multi_ref.cpp\n"
cat $PWD/multi_ref.cpp
echo -e "\n$ $CFG_PATH/a.out multi_ref config.map 0x400557 0x40056f cfg_test.txt\n"
$CFG_PATH/a.out $PWD/multi_ref $PWD/config.map 0x400557 0x40056f $PWD/cfg_test.txt
echo -e "$ cat cfg_test.txt"
cat $PWD/cfg_test.txt
echo -e "\n\n$ ./cfg_load cfg_test.txt"
$CFG_TEST_PATH/cfg_load $PWD/cfg_test.txt
