TARGET_EXEC = engine
CC = clang++

SRC = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(wildcard src/**/**/*.cpp) $(wildcard src/**/**/**/*.cpp)
OBJ = $(SRC:.cpp=.o)
ASM = $(SRC:.cpp=.S)
BIN = bin
LIBS = lib/bgfx/.build/linux64_gcc/bin/libbgfxDebug.a lib/bgfx/.build/linux64_gcc/bin/libbxDebug.a lib/bgfx/.build/linux64_gcc/bin/libbimgDebug.a lib/bgfx/.build/linux64_gcc/bin/libbimg_decodeDebug.a lib/glfw/build/src/libglfw3.a -lGL -lX11 -lpthread -ldl -lrt

INC_DIR_SRC = -Isrc 
INC_DIR_LIB = -Ilib/bgfx/include -Ilib/bimg/include -Ilib/bx/include -Ilib/glfw/include -Ilib/OBJ-Loader/include -Ilib/cgltf/include -Ilib/glm/include

DEBUGFLAGS = $(INC_DIR_SRC) $(INC_DIR_LIB) -Wall -g
RELEASEFLAGS = $(INC_DIR_SRC) $(INC_DIR_LIB) -O2
ASMFLAGS = $(INC_DIR_SRC) $(INC_DIR_LIBS) -Wall
LDFLAGS = $(LIBS) -lm -fuse-ld=mold

.PHONY: all clean  

all: clean shaders
	$(MAKE) -j8 bld
	$(MAKE) link

dirs:
	mkdir -p ./$(BIN)

link: $(OBJ)
	$(CC) -o $(BIN)/$(TARGET_EXEC) $^ $(LDFLAGS)

bld: 
	$(MAKE) dirs
	$(MAKE) obj

obj: $(OBJ)

asm: cleanassembly $(ASM)

%.o: %.cpp
	$(CC) -std=c++20 -o $@ -c $< $(DEBUGFLAGS)

%.S: %.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	@echo 'Building ASM'
	$(CC) -std=c++20 -S -O -o $@ -c $< $(ASMFLAGS)
	@echo 'Finished building: $<'
	@echo ' '

%.S: %.c 
	./$(BIN)/$(TARGET_EXEC) $< $@

shaders:
	$(MAKE) -C resources/shaders

run:
	./$(BIN)/$(TARGET_EXEC) 

clean:
	clear
	rm -rf $(BIN) $(OBJ)

cleanassembly:
	rm -rf $(ASM)