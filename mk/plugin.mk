include $(PROJECT_ROOT)/mk/common.mk

COMPONENT_PATH := $(patsubst $(PROJECT_ROOT)/%,%,$(CURDIR))

RUNTIME_TARGET := $(BUILD_DIR)/lib/$(COMPONENT_PATH)/libap-$(NAME).a
CONFIG_TARGET  := $(BUILD_DIR)/lib/$(COMPONENT_PATH)/config/libap-$(NAME).a


RUNTIME_OBJS := \
	$(RUNTIME_SRCS:%.c=$(BUILD_DIR)/$(NAME)/%.o)

CONFIG_OBJS := \
	$(CONFIG_SRCS:%.c=$(BUILD_DIR)/$(NAME)-config/%.o)


.PHONY: all clean

all: $(RUNTIME_TARGET) $(CONFIG_TARGET)


$(RUNTIME_TARGET): $(RUNTIME_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^


$(CONFIG_TARGET): $(CONFIG_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^


$(BUILD_DIR)/$(NAME)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS_COMMON) -c $< -o $@


$(BUILD_DIR)/$(NAME)-config/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) \
		$(CPPFLAGS) \
		$(addprefix -I,$(CONFIG_INC_DIRS)) \
		$(CFLAGS_COMMON) \
		$(CONFIG_CFLAGS) \
		-c $< -o $@


clean:
	rm -rf $(BUILD_DIR)/$(NAME)
	rm -rf $(BUILD_DIR)/$(NAME)-config
	rm -f $(RUNTIME_TARGET)
	rm -f $(CONFIG_TARGET)