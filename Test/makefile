
TRITON_PATH      = ../../Triton
LOADER_PATH      = $(TRITON_PATH)/binary_loader
DISASM_PATH      = $(TRITON_PATH)/disasm_util
TRITON_UTIL_PATH = $(TRITON_PATH)/triton_util
CFG_PATH         = $(TRITON_PATH)/cfg

BOOST_PATH = /home/linuxbrew/.linuxbrew/Cellar/boost/1.72.0_2
BOOST_LIB_PATH = $(BOOST_PATH)/lib

CC=gcc-4.9
CXX=g++-4.9
CXXFLAGS=-Wall -Wno-unknown-pragmas -Wno-unused-variable -O2 -std=c++11 -fno-stack-protector

.PHONY: all cfg_test cfg_load

all: branch code_coverage.o main cfg_test cfg_load test01

branch : branch/branch.c
	cd branch && $(CC) -o branch branch.c

code_coverage.o : code_coverage/code_coverage.cc
	cd code_coverage && $(CXX) -std=c++11 -c code_coverage.cc

main : code_coverage/main.cc
	cd code_coverage && $(CXX) $(CXXFLAGS) -o main main.cc code_coverage.o $(LOADER_PATH)/loader.a \
	$(DISASM_PATH)/disasm_util.a $(TRITON_UTIL_PATH)/triton_util.a $(CFG_PATH)/cfg.a \
	-ltriton -lbfd -lcapstone

cfg_test : cfg_test/cfg_test.cc
	cd cfg_test && export LD_LIBRARY_PATH=$(BOOST_PATH)/lib && \
	$(CXX) $(CXXFLAGS) -L$(BOOST_LIB_PATH) -o cfg_test cfg_test.cc \
	$(TRITON_PATH)/cfg_generation.a $(LOADER_PATH)/loader.a $(DISASM_PATH)/disasm_util.a \
	$(TRITON_UTIL_PATH)/triton_util.a $(CFG_PATH)/cfg.a \
	-ltriton -lbfd -lcapstone -lboost_serialization 

cfg_load : cfg_test/cfg_load.cc
	cd cfg_test && $(CXX) $(CXXFLAGS) -L$(BOOST_LIB_PATH) -o cfg_load cfg_load.cc \
	$(TRITON_PATH)/cfg_generation.a $(LOADER_PATH)/loader.a $(DISASM_PATH)/disasm_util.a \
	$(TRITON_UTIL_PATH)/triton_util.a $(CFG_PATH)/cfg.a \
	-ltriton -lbfd -lcapstone -lboost_serialization 

test01 : test01/test01.c
	cd test01 && $(CC) -o test01 test01.c

clean:
	rm -f branch *.o

