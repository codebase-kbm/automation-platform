AP_COMPONENT := yes
AP_TYPE := plugin

NAME := can


RUNTIME_SRCS := \
	$(wildcard src/*.c)


CONFIG_SRCS := \
	$(wildcard c-config/*.c)


INC_DIRS := \
	include \
	$(PROJECT_ROOT)/core/include


CONFIG_INC_DIRS := \
	include \
	$(PROJECT_ROOT)/core/include \
	$(PROJECT_ROOT)/apps/linux/config-compiler/include


CONFIG_CFLAGS += \
	$(shell pkg-config --cflags libxml-2.0)


include $(PROJECT_ROOT)/mk/plugin.mk