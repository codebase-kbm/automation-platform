AP_COMPONENT := yes
AP_TYPE := plugin

NAME := peer-server


RUNTIME_SRCS := \
	$(wildcard src/*.c)\
	$(wildcard $(PROJECT_ROOT)/peer-protocol/src/*.c)



CONFIG_SRCS := \
	c-config/peer_server.c


INC_DIRS := \
	include \
	$(PROJECT_ROOT)/peer-protocol/include \
	$(PROJECT_ROOT)/adapters/api \
	$(PROJECT_ROOT)/core/include


CONFIG_INC_DIRS := \
	include \
	$(PROJECT_ROOT)/core/include \
	$(PROJECT_ROOT)/apps/linux/config-compiler/include


CONFIG_CFLAGS += \
	$(shell pkg-config --cflags libxml-2.0)


include $(PROJECT_ROOT)/mk/plugin.mk