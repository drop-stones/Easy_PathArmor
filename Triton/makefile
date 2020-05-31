
BOOST_INC_PATH = /home/linuxbrew/.linuxbrew/Cellar/boost/1.72.0_2/include/

CC=gcc-4.9
CXX=g++-4.9
CXXFLAGS=-Wall -Wno-unknown-pragmas -Wno-unused-variable -O2 -std=c++11 -fno-stack-protector -I$(TRITON_HOME)/src/libtriton/includes/ -L$(TRITON_HOME)/build/src/libtriton/

.PHONY: all clean

all: loader.o loader.a triton_util.o triton_util.a disasm_util.o disasm_util.a cfg.o cfg.a cfg_generation.o cfg_generation.a

loader.o: binary_loader/loader.cc
	cd binary_loader && $(CXX) -std=c++11 -g -c -fPIC loader.cc

loader.a: binary_loader/loader.o
	cd binary_loader && ar -r loader.a loader.o

triton_util.o: triton_util/triton_util.cc
	cd triton_util && $(CXX) $(CXXFLAGS) -std=c++11 -g -c -fPIC triton_util.cc

triton_util.a: triton_util/triton_util.o
	cd triton_util && ar -r triton_util.a triton_util.o

disasm_util.o: disasm_util/disasm_util.cc
	cd disasm_util && $(CXX) -std=c++11 -g -c -fPIC disasm_util.cc

disasm_util.a: disasm_util/disasm_util.o
	cd disasm_util && ar -r disasm_util.a disasm_util.o

cfg.o: cfg/cfg.cc
	cd cfg && $(CXX) -std=c++11 -g -c -fPIC -I$(BOOST_INC_PATH) cfg.cc

cfg.a: cfg/cfg.o
	cd cfg && ar -r cfg.a cfg.o

cfg_generation.o: cfg_generation.cc loader.o triton_util.o disasm_util.o
	$(CXX) $(CXXFLAGS) -c -fPIC cfg_generation.cc

cfg_generation.a: cfg_generation.o
	ar -r cfg_generation.a cfg_generation.o

clean:
	rm -f cfg_generation *.o

