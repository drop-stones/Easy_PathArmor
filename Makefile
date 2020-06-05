PIN_ROOT=/home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux
PIN_HOME=/home/binary/triton/pin-2.14-71313-gcc.4.4.7-linux
TRITON_HOME=$(PIN_HOME)/source/tools/Triton

CFG_SRC=cfg_generation/src

.PHONY: all clean cfg_generation kernel_module Test

all: cfg_generation kernel_module Test

cfg_generation: cfg_generation/makefile $(CFG_SRC)/cfg_generation.cpp $(CFG_SRC)/loader.cpp $(CFG_SRC)/disasm_util.cpp $(CFG_SRC)/triton_util.cpp $(CFG_SRC)/cfg.cpp
	export TRITON_HOME=$(TRITON_HOME) && cd cfg_generation && $(MAKE)

kernel_module : kernel_module/makefile kernel_module/src/kernel_module.cpp
	export PIN_ROOT=$(PIN_ROOT) && cd kernel_module && $(MAKE)

Test : Test/makefile Test/branch/branch.c Test/cfg_test/cfg_load.cpp Test/test01/test01.c
	export TRITON_HOME=$(TRITON_HOME) && cd Test && $(MAKE)

clean :
	cd cfg_generation && $(MAKE) clean
	export PIN_ROOT=$(PIN_ROOT) && cd kernel_module  && $(MAKE) clean
	cd Test           && $(MAKE) clean
