AP_COMPONENT := yes
NAME := config-compiler


SRCS := \
	$(wildcard src/*.c)

INC_DIRS := \
	include \
	$(PROJECT_ROOT)/core/include


CFLAGS += \
	$(shell pkg-config --cflags libxml-2.0)


LDLIBS += \
	$(shell pkg-config --libs libxml-2.0)

CORE_LIBS := \
	$(BUILD_DIR)/lib/core/libap-core.a

PLUGIN_COMPILER_LIBS := \
    $(wildcard $(BUILD_DIR)/lib/plugins/*/config/libap-*.a)

$(info PLUGIN_COMPILER_LIBS=$(PLUGIN_COMPILER_LIBS))

LDFLAGS += \
    -Wl,--whole-archive $(PLUGIN_COMPILER_LIBS) \
    -Wl,--no-whole-archive \
    $(CORE_LIBS)

include $(PROJECT_ROOT)/mk/binary.mk