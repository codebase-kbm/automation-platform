AP_COMPONENT := yes
AP_DEPENDS := core
NAME := ap-runner

SRCS := \
	main.c \
	$(PROJECT_ROOT)/apps/linux/common/src/*.c

INC_DIRS := \
	$(PROJECT_ROOT)/core/include \
	$(PROJECT_ROOT)/apps/linux/common/include \
	$(PROJECT_ROOT)/core/defs


CORE_LIBS := \
    $(BUILD_DIR)/lib/core/libap-core.a

PLUGIN_LIBS := \
	$(wildcard $(BUILD_DIR)/lib/plugins/*/libap-*.a)


ADAPTER_INFO := \
	$(wildcard $(BUILD_DIR)/lib/adapters/linux/*/component.mk)

include $(ADAPTER_INFO)

ADAPTER_LIBS := $(AP_ADAPTER_LIBS)

LDLIBS += $(AP_ADAPTER_LDLIBS)
LDLIBS += $(AP_ADAPTER_DEPENDS)


LDFLAGS += \
    -Wl,--whole-archive $(PLUGIN_LIBS) \
    -Wl,--no-whole-archive \
    $(ADAPTER_LIBS) \
    $(CORE_LIBS)

include $(PROJECT_ROOT)/mk/binary.mk