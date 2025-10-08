# Directories
SRC    := ./src
BIN    := ./bin
BUILD  := ./build



# Objects inside the build directory
MAIN_OBJ := $(BUILD)/main.o
INIT_OBJ := $(BUILD)/init.o
HASH_OBJ := $(BUILD)/hash_objects.o 
CAT_OBJ := $(BUILD)/cat_file.o

# Objects for executable build
OBJ := $(MAIN_OBJ) $(INIT_OBJ) $(HASH_OBJ) $(CAT_OBJ)

# GCC flags
FLAGS := -lz

# Default target
.PHONY: all
all: $(BIN)/wyag



# Build final binary
$(BIN)/wyag: $(INIT_OBJ) $(HASH_OBJ) $(MAIN_OBJ) | $(BIN)
	gcc $(OBJ) $(FLAGS) -o $@
	echo "LD_LIBRARY_PATH=./lib ./bin/wyag"

# Compile main.c to object in build dir
$(MAIN_OBJ): $(SRC)/main.c | $(BUILD)
	gcc -c $(SRC)/main.c -o $@

# Build objects 
$(INIT_OBJ): $(SRC)/init.c | $(BUILD)
	gcc -c $(SRC)/init.c -o $(INIT_OBJ)

$(HASH_OBJ): $(SRC)/hash_objects.c | $(BUILD)
	gcc -c $(SRC)/hash_objects.c -o $(HASH_OBJ)

$(CAT_OBJ): $(SRC)/cat_file.c | $(BUILD)
	gcc -c $(SRC)/cat_file.c -o $(CAT_OBJ)

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
