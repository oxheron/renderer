TARGET_EXEC = libRenderer.a
CC = clang++
LD = ar

DEBUG = 1

SRC = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(wildcard src/**/**/*.cpp) $(wildcard src/**/**/**/*.cpp)
OBJ = $(SRC:.cpp=.o)
BIN = bin

INC_DIR_SRC = -Isrc 
INC_DIR_LIB = -Ilib -Ilib/json/single_include -Ilib/bgfx/include -Ilib/bimg/include -Ilib/bx/include -Ilib/glfw/include -Ilib/cgltf/include -Ilib/glm -Ilib/robin-hood-hashing/src/

DEBUGFLAGS = $(INC_DIR_SRC) $(INC_DIR_LIB) -Wall -g -DDEBUG=1 -DBX_CONFIG_DEBUG=1
RELEASEFLAGS = $(INC_DIR_SRC) $(INC_DIR_LIB) -O2 -DBX_CONFIG_DEBUG=0
ifeq ($(DEBUG), 1)
	CFLAGS = $(DEBUGFLAGS)
else
	CFLAGS = $(RELEASEFLAGS)
endif
LDFLAGS = rcs 

.PHONY: all clean  

all: clean 
	$(MAKE) -j8 bld
	$(MAKE) link

dirs:
	mkdir -p ./$(BIN)

link: $(OBJ)
	$(AR) $(LDFLAGS) $(BIN)/$(TARGET_EXEC) $^ 

bld: 
	$(MAKE) dirs
	$(MAKE) obj

obj: $(OBJ)

%.o: %.cpp
	$(CC) -std=c++20 -o $@ -c $< $(CFLAGS)

headers:
	python3 scripts/headers_to_lib.py

clean:
	clear
	rm -rf $(BIN) $(OBJ)
