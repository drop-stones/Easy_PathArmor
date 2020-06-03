
CC=gcc-4.9
CXX=g++-4.9
CXXFLAGS=-Wall -Wno-unknown-pragmas -Wno-unused-variable -O2 -std=c++11 -fno-stack-protector -I$(TRITON_HOME)/src/libtriton/includes/ -L$(TRITON_HOME)/build/src/libtriton/

.PHONY: all clean

all: obj loader.o triton_util.o disasm_util.o cfg.o cfg_generation.o a.out

loader.o: src/loader.cpp
	$(CXX) -std=c++11 -g -c -o obj/loader.o src/loader.cpp

triton_util.o: src/triton_util.cpp
	$(CXX) -std=c++11 -g -c -o obj/triton_util.o src/triton_util.cpp

disasm_util.o: src/disasm_util.cpp
	$(CXX) -std=c++11 -g -c -o obj/disasm_util.o src/disasm_util.cpp

cfg.o: src/cfg.cpp
	$(CXX) -std=c++11 -g -c -o obj/cfg.o src/cfg.cpp

cfg_generation.o: src/cfg_generation.cpp
	$(CXX) $(CXXFLAGS) -c -o obj/cfg_generation.o src/cfg_generation.cpp

a.out: src/main.cpp obj/loader.o obj/triton_util.o obj/disasm_util.o obj/cfg.o obj/cfg_generation.o
	$(CXX) -std=c++11 src/main.cpp obj/loader.o obj/triton_util.o obj/disasm_util.o \
	obj/cfg.o obj/cfg_generation.o \
	-ltriton -lbfd -lcapstone

obj:
	mkdir -p obj

clean:
	rm -f cfg_generation *.o

