#!/bin/bash

PIN_ROOT=/home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux
PATH_ARMOR=/home/binary/code/PathArmor
KERNEL_MODULE_OBJ=$PATH_ARMOR/kernel_module/obj
CFG_PATH=$PATH_ARMOR/cfg_generation
TEST_PATH=$PATH_ARMOR/Test
TEST01_PATH=$TEST_PATH/test01

ENTRY=0x40065b
EXIT=0x40066e

echo -e "$ cat test01.c\n"
cat $TEST01_PATH/test01.c

echo -e "$ $CFG_PATH/a.out test01 config.map $ENTRY $EXIT cfg.txt\n"
$CFG_PATH/a.out $TEST01_PATH/test01 $TEST01_PATH/config.map $ENTRY $EXIT $TEST01_PATH/cfg.txt

echo -e "\n$ $PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- test01 0 $ENTRY $EXIT cfg.txt"
$PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- \
  $TEST01_PATH/test01 0 $ENTRY $EXIT $TEST01_PATH/cfg.txt

echo -e "\n$ $PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- test01_attacked 0 $ENTRY $EXIT cfg.txt"
$PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- \
  $TEST01_PATH/test01_attacked 0 $ENTRY $EXIT $TEST01_PATH/cfg.txt
