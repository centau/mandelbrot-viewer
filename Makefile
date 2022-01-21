# Macros
CC = g++

INCLUDE_DIR = src/include
BIN_DIR = bin
LIB_DIR = src/lib

HEADER_FILES = $(wildcard $(INCLUDE_DIR)/*.hpp)
SRC_FILES = $(wildcard src/*.cpp)
BIN_FILES = $(patsubst src/%.cpp,$(BIN_DIR)/%.o,$(SRC_FILES))

CFLAGS = -I $(INCLUDE_DIR) -L $(LIB_DIR) -l sfml-window -l sfml-system -l sfml-graphics -O3

# Rules
$(BIN_DIR)/%.o: src/%.cpp $(HEADER_FILES)
	$(CC) -o $@ -c $< $(CFLAGS)

build: Main.exe
	@echo Compiled and linked!

build_run: build
	@echo ============================
	@echo Executing program...
	@echo ============================
	@./bin/Main.exe
	@echo ============================
	@echo Execution complete!
	@echo ============================

Main.exe: $(BIN_FILES)
	$(CC) -o bin/$@ $^ $(CFLAGS)

.PHONY: clean

# only works for cmd
clean:
	del src\bin\*.o