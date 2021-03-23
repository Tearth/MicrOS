GCC := /opt/cross/bin/i386-elf-gcc

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
OUTPUT_FILE := $(BIN_DIR)/shell.elf

C_SOURCES := $(shell find $(SRC_DIR) -name "*.c")
C_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))

ASM_SOUCES := $(shell find $(SRC_DIR) -name "*.asm")
ASM_OBJECTS := $(ASM_SOUCES:%.asm=$(OBJ_DIR)/%.o)

DEPS := $(C_OBJECTS:.o=.d)

GCC_COMPILE_FLAGS := -I../../library/src -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-packed-bitfield-compat
GCC_LINK_FLAGS := -L../../library/bin -lc -lgcc -nostdlib

ifeq ($(MODE),release)
GCC_COMPILE_FLAGS += -O2
GCC_LINK_FLAGS += -s -DNDEBUG
else
GCC_COMPILE_FLAGS += -O0 -ggdb3
endif

$(OUTPUT_FILE): $(C_OBJECTS) $(ASM_OBJECTS)
	@echo "=== Linking output file $(OUTPUT_FILE) ==="
	@mkdir -p '$(@D)'
	@$(GCC) $(C_OBJECTS) -o $(OUTPUT_FILE) $(GCC_LINK_FLAGS)
	@echo "=== $(OUTPUT_FILE) done ==="

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "=== Compiling C file $< into object $@ ==="
	@mkdir -p '$(@D)'
	@$(GCC) -c $(PWD)/$< -o $@ -MMD -MF $(@:.o=.d) $(GCC_COMPILE_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo "=== Compiling asm file $< into object $@ ==="
	@mkdir -p '$(@D)'
	@$(NASM) $(PWD)/$< -f elf32 -o $@

-include $(DEPS)

clean:
	@echo "=== Clearing $(BIN_DIR) and $(OBJ_DIR) directories ==="
	rm -r -f $(BIN_DIR) $(OBJ_DIR)
	@echo "=== Done ==="