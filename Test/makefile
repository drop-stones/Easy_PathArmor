
CFG_GEN_PATH = /home/binary/code/PathArmor/cfg_generation
CFG_SRC      = $(CFG_GEN_PATH)/src
CFG_OBJ      = $(CFG_GEN_PATH)/obj

CC=gcc-4.9
CXX=g++-4.9
CXXFLAGS=-Wall -Wno-unknown-pragmas -Wno-unused-variable -O2 -std=c++11 -fno-stack-protector

.PHONY: all cfg_test cfg_load bb_divide loop test01 test02 test03 test04

all: branch cfg_load multi_ref bb_divide loop test01 test01_attacked test02 test02_attacked test03 \
     test03_attacked test04 test04_attacked

branch: branch/branch.c
	cd branch && $(CC) -o branch branch.c

cfg_load: cfg_test/cfg_load.cpp
	cd cfg_test && $(CXX) -o cfg_load cfg_load.cpp $(CFG_OBJ)/cfg.o

multi_ref: cfg_test/multi_ref/multi_ref.cpp
	cd cfg_test/multi_ref && $(CXX) -o multi_ref multi_ref.cpp

bb_divide: cfg_test/bb_divide/bb_divide.cpp
	cd cfg_test/bb_divide && $(CXX) -o bb_divide bb_divide.cpp

loop: cfg_test/loop/loop.cpp
	cd cfg_test/loop && $(CXX) -o loop loop.cpp

test01: test01/test01.s
	cd test01 && $(CC) -o test01 test01.s

test01_attacked: test01/test01_attacked.s
	cd test01 && $(CC) -o test01_attacked test01_attacked.s

test02: test02/test02.s
	cd test02 && $(CC) -o test02 test02.s

test02_attacked: test02/test02_attacked.s
	cd test02 && $(CC) -o test02_attacked test02_attacked.s

test03: test03/test03.s
	cd test03 && $(CC) -o test03 test03.s

test03_attacked: test03/test03_attacked.s
	cd test03 && $(CC) -o test03_attacked test03_attacked.s

test04: test04/test04.s
	cd test04 && $(CC) -o test04 test04.s

test04_attacked: test04/test04_attacked.s
	cd test04 && $(CC) -o test04_attacked test04_attacked.s

clean:
	rm -f branch/branch cfg_test/cfg_load cfg_test/multi_ref/multi_ref cfg_test/multi_ref/cfg.txt \
	      cfg_test/test01/cfg.txt cfg_test/bb_divide/bb_divide cfg_test/bb_divide/cfg.txt \
	      cfg_test/loop/loop cfg_test/loop/cfg.txt test01/test01 test01/test01_attacked test01/cfg.txt \
	      test02/test02 test02/test02_attacked test02/cfg.txt \
	      test03/test03 test03/test03_attacked test03_cfg.txt \
	      test04/test04 test04/test04_attacked test04/cfg.txt

