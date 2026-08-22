AP_COMPONENT := yes
AP_TYPE := adapter
NAME := libmosquitto
API := mqtt
PLATFORM := linux
DEPENDS_LIBS := libmosquitto

SRCS := \
	$(wildcard *.c)

INC_DIRS := \
	$(PROJECT_ROOT)/plugins/mqtt/include \
	$(PROJECT_ROOT)/core/include

LDLIBS += $(shell pkg-config --libs $(DEPENDS_LIBS))

include $(PROJECT_ROOT)/mk/adapter.mk