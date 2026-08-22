# --------------------------------------------------
# Common build settings
# --------------------------------------------------

PROJECT_ROOT ?= $(abspath $(CURDIR)/..)

BUILD_DIR ?= $(PROJECT_ROOT)/build


CC ?= gcc
AR ?= ar


CFLAGS += \
	-std=c11 \
	-D_POSIX_C_SOURCE=200809L \
	-Wall \
	-Wextra \
	-Wpedantic \
	-g


CPPFLAGS += \
	$(addprefix -I,$(INC_DIRS))