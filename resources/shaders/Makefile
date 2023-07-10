CC = ../../lib/bgfx/.build/linux64_gcc/bin/shadercRelease

VERTEX_SRC = $(wildcard *.vs)
FRAGMENT_SRC = $(wildcard *.fs)
COMPUTE_SRC = $(wildcard *.cs)

VERTEX_BIN = $(VERTEX_SRC:.vs=.vs.bin)
FRAGMENT_BIN = $(FRAGMENT_SRC:.fs=.fs.bin)
COMPUTE_BIN = $(COMPUTE_SRC:.cs=.cs.bin)

OUTPUT = spirv 440 

ifeq ($(shell uname),Windows_NT)
	OUTPUT += s_5_0
endif

ifeq ($(shell uname),Darwin)
	OUTPUT += metal
endif

BIN = bin

.PHONY: all clean

all : clean dirs $(VERTEX_BIN) $(FRAGMENT_BIN) $(COMPUTE_BIN)

dirs:
	$(foreach var,$(OUTPUT),mkdir -p $(var);)

%.vs.bin: %.vs
	$(foreach var,$(OUTPUT),$(CC) -f $< -o ./$(var)/$@ --type vertex -p $(var) -i include;)

%.fs.bin: %.fs
	$(foreach var,$(OUTPUT),$(CC) -f $< -o ./$(var)/$@ --type fragment -p $(var) -i include;)

%.cs.bin: %.cs
	$(foreach var,$(OUTPUT),$(CC) -f $< -o ./$(var)/$@ --type compute -p $(var) -i include;)

clean:
	$(foreach var,$(OUTPUT),rm -rf $(var);)
