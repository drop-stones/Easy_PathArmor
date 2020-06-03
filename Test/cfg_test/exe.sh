#!/bin/bash

PATH_ARMOR=/home/binary/code/PathArmor
CFG_PATH=$PATH_ARMOR/cfg_generation
TEST_PATH=$PATH_ARMOR/Test
CFG_TEST_PATH=$TEST_PATH/cfg_test

echo -e "$ $CFG_PATH/a.out\n"
$CFG_PATH/a.out $TEST_PATH/test01/test01 $TEST_PATH/test01/configure.map 0x400746 0x40075e \
   $CFG_TEST_PATH/cfg_test.txt
echo -e "$ cat cfg_test.txt cfg_test.txt"
cat $CFG_TEST_PATH/cfg_test.txt
echo -e "\n\n$ ./cfg_load"
$TEST_PATH/cfg_test/cfg_load $CFG_TEST_PATH/cfg_test.txt
