
CFG_GEN_PATH = /home/binary/code/PathArmor/cfg_generation
CFG_SRC      = $(CFG_GEN_PATH)/src
CFG_OBJ      = $(CFG_GEN_PATH)/obj

CC=gcc-4.9
CXX=g++-4.9
CXXFLAGS=-Wall -Wno-unknown-pragmas -Wno-unused-variable -O2 -std=c++11 -fno-stack-protector

.PHONY: all cfg_test cfg_load

all: branch cfg_load test01

branch : branch/branch.c
	cd branch && $(CC) -o branch branch.c

cfg_load : cfg_test/cfg_load.cpp
	cd cfg_test && $(CXX) -o cfg_load cfg_load.cpp $(CFG_OBJ)/cfg.o

test01 : test01/test01.c
	cd test01 && $(CC) -o test01 test01.c

clean:
	rm -f branch *.o

