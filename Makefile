# Compiler
CC := gcc

# Projektname
TARGET := automation-platform

# Verzeichnisse
CORE_SRC := \
    $(wildcard core/src/*.c)
	
CONFIG_CORE_SRC := \
    core/src/ap_config_reader.c \
    core/src/ap_core.c \
    core/src/ap_dispatcher.c \
    core/src/ap_event.c \
    core/src/ap_registry.c \
    core/src/ap_timestamp.c
	
SRC_DIRS := \
    core/src \
    examples/linux/ap-runner \
    examples/linux/common \
    $(wildcard plugins/*) \
    $(wildcard adapters/linux/*/)

INC_DIRS := \
    core/include \
    examples/linux/common \
    $(wildcard plugins/*/) \
    $(wildcard adapters/linux/*/) \
    tools/config-compiler

BUILD_DIR := build

# Compiler Optionen
CFLAGS := -std=c11 \
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


# --------------------------------------------------
# Config Compiler
# --------------------------------------------------

CONFIG_COMPILER_SRC := \
    $(wildcard tools/config-compiler/*.c) \
    $(wildcard plugins/*/c-config/*.c) \
    $(CONFIG_CORE_SRC)

CONFIG_COMPILER := $(BUILD_DIR)/config-compiler

CONFIG_XML := config/config.xml
CONFIG_BIN := $(BUILD_DIR)/config.bin

CONFIG_CFLAGS := $(CFLAGS) \
                 $(shell pkg-config --cflags libxml-2.0)

CONFIG_LIBS := $(shell pkg-config --libs libxml-2.0)

# --------------------------------------------------
# Plugin Test
# --------------------------------------------------

PLUGIN_TEST_SRC := \
    examples/demo-plugin/main.c \
    core/src/ap_plugin_manager.c \
    core/src/ap_config_reader.c \
    plugins/demo/demo_plugin.c

PLUGIN_TEST := $(BUILD_DIR)/demo-plugin


$(PLUGIN_TEST): $(PLUGIN_TEST_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@


dummy-plugin: $(PLUGIN_TEST)
	./$(PLUGIN_TEST)

# --------------------------------------------------
# Standardziel
# --------------------------------------------------

all: $(BUILD_DIR)/$(TARGET) $(CONFIG_COMPILER) $(CONFIG_BIN)


# --------------------------------------------------
# Hauptprogramm
# --------------------------------------------------

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)


# --------------------------------------------------
# Kompilieren
# --------------------------------------------------

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# --------------------------------------------------
# Config Compiler
# --------------------------------------------------

$(CONFIG_COMPILER): $(CONFIG_COMPILER_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CONFIG_CFLAGS) $^ -o $@ $(CONFIG_LIBS)

config-compiler: $(CONFIG_COMPILER)

$(CONFIG_BIN): $(CONFIG_XML) $(CONFIG_COMPILER)
	./$(CONFIG_COMPILER) $(CONFIG_XML) $(CONFIG_BIN)


config: $(CONFIG_BIN)


# --------------------------------------------------
# Config test tools
# --------------------------------------------------

config-dump: tools/config-dump/main.c core/src/ap_config_reader.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@

# --------------------------------------------------
# Cleanup
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(CONFIG_BIN)

rebuild: clean all

run: all
	./$(BUILD_DIR)/$(TARGET)


.PHONY: all clean rebuild run config config-dump config-compiler demo-plugin