# Directories
SRC_SRC    := ../src
SRC := .
BIN    := ../bin
BUILD  := ../build



# Objects inside the build directory
MAIN_OBJ := $(BUILD)/main.o
INIT_OBJ := $(BUILD)/init.o
CAT_OBJ := $(BUILD)/cat_file.o

# Objects for executable build
OBJ := $(MAIN_OBJ) $(INIT_OBJ) $(CAT_OBJ)

# GCC flags
FLAGS := -lz -lcrypto

# Default target
.PHONY: all
all: $(BIN)/wyag



# Build final binary
$(BIN)/wyag: $(INIT_OBJ) $(CAT_OBJ) $(MAIN_OBJ) | $(BIN)
	gcc $(OBJ) $(FLAGS) -o $@
	echo "LD_LIBRARY_PATH=./lib ./bin/wyag"

# Compile main.c to object in build dir
$(MAIN_OBJ): $(SRC_SRC)/main.c | $(BUILD)
	gcc -c $(SRC)/test_one.c -o $@ -g

# Build objects 
$(INIT_OBJ): $(SRC_SRC)/init.c | $(BUILD)
	gcc -c $(SRC_SRC)/init.c -o $(INIT_OBJ) -g

$(CAT_OBJ): $(SRC_SRC)/cat_file.c | $(BUILD)
	gcc -c $(SRC_SRC)/cat_file.c -o $(CAT_OBJ) -g

# Directories creation
$(BUILD):
	mkdir -p $(BUILD)

$(LIB):
	mkdir -p $(LIB)

$(BIN):
	mkdir -p $(BIN)


# Convenience targets
.PHONY: clean
clean:
	rm -rf $(BUILD)/*.o $(BIN)/wyag


.PHONY: install
install: $(BIN)/wyag
	cp $(BIN)/wyag /usr/local/bin/ || true
