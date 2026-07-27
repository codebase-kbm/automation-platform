# Compiler
CC      := gcc

# Projektname
TARGET  := automation-platform

# Verzeichnisse
SRC_DIRS := \
	core/src \
	adapters/logger \
	examples/minimal \
	adapters/mqtt

INC_DIRS := \
	core/include \
	adapters/logger \
	adapters/mqtt

BUILD_DIR := build

# Compiler Optionen
CFLAGS  := -std=c11 \
           -D_POSIX_C_SOURCE=200809L \
           -Wall \
           -Wextra \
           -Wpedantic \
           -g \
           $(addprefix -I,$(INC_DIRS))

# Linker
LDFLAGS := -lmosquitto -lcjson

# Quellen automatisch finden
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Objektdateien
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Standardziel
all: $(BUILD_DIR)/$(TARGET)

# Linken
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Kompilieren
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Aufräumen
clean:
	rm -rf $(BUILD_DIR)

# Neu bauen
rebuild: clean all

# Ausführen
run: all
	./$(BUILD_DIR)/$(TARGET)

.PHONY: all clean rebuild run