# Project name
PROJ_NAME=C_ThreadPool

# Directories
SRC_DIR=./src
BIN_DIR=./bin
INCLUDE_DIR=$(SRC_DIR)/include

# .c files
C_SOURCE=$(wildcard $(SRC_DIR)/*.c)
 
# .h files
H_SOURCE=$(wildcard $(INCLUDE_DIR)/*.h)
 
# Object files
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(C_SOURCE))
 
# Compiler
CC=gcc

# C standart to use
STD=c17

# Flags for compiler
CC_FLAGS=-c         \
		 -g			\
         -W         \
         -Wall      \
		 -lm		\
		 -Wextra	\
		 -lpthread	\
		 -std=$(STD)

# The main file (with the main() function)
MAIN_FILE=main.c
MAIN_OBJECT=$(BIN_DIR)/$(MAIN_FILE:.c=.o)

#
# Compilation and linking
#
all: $(PROJ_NAME)
 
$(PROJ_NAME): $(OBJ) $(MAIN_OBJECT)
	$(CC) -o $(BIN_DIR)/$@ $^
 
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
	$(CC) -o $@ $< $(CC_FLAGS)
 
$(MAIN_OBJECT): $(MAIN_FILE) $(H_SOURCE)
	$(CC) -o $@ $< $(CC_FLAGS)
 
clean:
	rm -rf $(BIN_DIR)/*.o $(BIN_DIR)/$(PROJ_NAME) $(BIN_DIR)/*~

run: $(PROJ_NAME)
	$(BIN_DIR)/$(PROJ_NAME)

release:
	$(MAKE) all CC_FLAGS='$(CC_FLAGS:-g=-O3)'

run_release:
	$(MAKE) run CC_FLAGS='$(CC_FLAGS:-g=-O3)'