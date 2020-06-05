#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_PATH=$PATH_ARMOR/cfg_generation
TEST_PATH=$PATH_ARMOR/Test
CFG_TEST_PATH=$TEST_PATH/cfg_test

#echo -e "$ $CFG_PATH/a.out $TEST_PATH/test01/test01 $TEST_PATH/test01/config.map 0x400746 0x40075e $CFG_TEST_PATH/cfg_test.txt\n"
#$CFG_PATH/a.out $TEST_PATH/test01/test01 $TEST_PATH/test01/config.map 0x400746 0x40075e \
#   $CFG_TEST_PATH/cfg_test.txt
#echo -e "$ cat cfg_test.txt"
#cat $CFG_TEST_PATH/cfg_test.txt
#echo -e "\n\n$ ./cfg_load cfg_test.txt"
#$CFG_TEST_PATH/cfg_load $CFG_TEST_PATH/cfg_test.txt

echo -e "$ cat multi_ref.cpp\n"
cat $CFG_TEST_PATH/multi_ref.cpp
echo -e "\n$ $CFG_PATH/a.out multi_ref config.map 0x400557 0x40056f cfg_test.txt\n"
$CFG_PATH/a.out $CFG_TEST_PATH/multi_ref $CFG_TEST_PATH/config.map 0x400557 0x40056f \
    $CFG_TEST_PATH/cfg_test.txt
echo -e "$ cat cfg_test.txt"
cat $CFG_TEST_PATH/cfg_test.txt
echo -e "\n\n$ ./cfg_load cfg_test.txt"
$CFG_TEST_PATH/cfg_load $CFG_TEST_PATH/cfg_test.txt
