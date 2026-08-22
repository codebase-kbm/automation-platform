# --------------------------------------------------
# Static library build rules
# --------------------------------------------------

MK_ROOT := $(dir $(lastword $(MAKEFILE_LIST)))

include $(MK_ROOT)/common.mk


COMPONENT_PATH := $(patsubst $(PROJECT_ROOT)/%,%,$(CURDIR))

TARGET := $(BUILD_DIR)/lib/$(COMPONENT_PATH)/libap-$(NAME).a


OBJS := \
	$(SRCS:%.c=$(BUILD_DIR)/$(NAME)/%.o)


.PHONY: all clean


all: $(TARGET)


$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^


$(BUILD_DIR)/$(NAME)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR)/$(NAME)
	rm -f $(TARGET)