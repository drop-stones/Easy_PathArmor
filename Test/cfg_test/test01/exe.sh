#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_PATH=$PATH_ARMOR/cfg_generation
TEST_PATH=$PATH_ARMOR/Test
CFG_TEST_PATH=$TEST_PATH/cfg_test
WD=$CFG_TEST_PATH/test01

echo -e "$ $CFG_PATH/a.out $TEST_PATH/test01/test01 $TEST_PATH/test01/config.map 0x400746 0x40075e $CFG_TEST_PATH/cfg_test.txt\n"
$CFG_PATH/a.out $TEST_PATH/test01/test01 $TEST_PATH/test01/config.map 0x400746 0x40075e $WD/cfg_test.txt
echo -e "$ cat cfg_test.txt"
cat $WD/cfg_test.txt
echo -e "\n\n$ ./cfg_load cfg_test.txt"
$CFG_TEST_PATH/cfg_load $WD/cfg_test.txt

