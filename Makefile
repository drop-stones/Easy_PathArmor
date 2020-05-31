PIN_ROOT=/home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux
PIN_HOME=/home/binary/triton/pin-2.14-71313-gcc.4.4.7-linux
TRITON_HOME=$(PIN_HOME)/source/tools/Triton

.PHONY: all clean Triton Pin Test

all: Triton Pin Test

Triton: Triton/makefile Triton/cfg_generation.cc Triton/binary_loader/loader.cc Triton/disasm_util/disasm_util.cc Triton/triton_util/triton_util.cc Triton/cfg/cfg.cc
	export TRITON_HOME=$(TRITON_HOME) && export PIN_ROOT=$(PIN_ROOT) && cd Triton && $(MAKE)

Pin : Pin/makefile Pin/src/track.cpp Pin/src/kernel_module.cpp
	export PIN_ROOT=$(PIN_ROOT) && cd Pin && $(MAKE)

Test : Test/makefile Test/branch/branch.c Test/code_coverage/code_coverage.cc Test/code_coverage/main.cc Test/cfg_test/cfg_test.cc Test/test01/test01.c
	export TRITON_HOME=$(TRITON_HOME) && cd Test && $(MAKE)

clean : 
