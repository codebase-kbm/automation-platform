AP_COMPONENT := yes
AP_TYPE := adapter
NAME := http
API := http
PLATFORM := linux
DEPENDS_LIBS := libcurl

SRCS := \
	$(wildcard *.c)

INC_DIRS := \
	$(PROJECT_ROOT)/plugins/influx/include \
	$(PROJECT_ROOT)/adapters/api \
	$(PROJECT_ROOT)/core/include


LDLIBS += $(shell pkg-config --libs $(DEPENDS_LIBS))

include $(PROJECT_ROOT)/mk/adapter.mk