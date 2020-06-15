#!/bin/bash

PIN_ROOT=/home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux
PATH_ARMOR=/home/binary/code/PathArmor
KERNEL_MODULE_OBJ=$PATH_ARMOR/kernel_module/obj
CFG_PATH=$PATH_ARMOR/cfg_generation
TEST_PATH=$PATH_ARMOR/Test

NAME=test02
WD=$TEST_PATH/$NAME
ENTRY=0x40069e
EXIT=0x4006b6

echo -e "$ cat $NAME.c\n"
cat $WD/$NAME.c

echo -e "$ $CFG_PATH/a.out $NAME config.map $ENTRY $EXIT cfg.txt\n"
$CFG_PATH/a.out $WD/$NAME $WD/config.map $ENTRY $EXIT $WD/cfg.txt

echo -e "\n$ $PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- $NAME 0 $ENTRY $EXIT cfg.txt"
$PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- \
  $WD/$NAME 0 $ENTRY $EXIT $WD/cfg.txt

echo -e "\n$ $PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- $NAME 1 $ENTRY $EXIT cfg.txt"
$PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- \
  $WD/$NAME 1 $ENTRY $EXIT $WD/cfg.txt

echo -e "\n$ $PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- ${NAME}_attacked 0 $ENTRY $EXIT cfg.txt"
$PIN_ROOT/pin -t $KERNEL_MODULE_OBJ/kernel_module.so -- \
  $WD/${NAME}_attacked 0 $ENTRY $EXIT $WD/cfg.txt
