# --------------------------------------------------
# Binary build rules
# --------------------------------------------------

MK_ROOT := $(dir $(lastword $(MAKEFILE_LIST)))

include $(MK_ROOT)/common.mk


TARGET := \
	$(BUILD_DIR)/bin/$(NAME)


OBJS := \
	$(SRCS:%.c=$(BUILD_DIR)/$(NAME)/%.o)


.PHONY: all clean


all: $(TARGET)


$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) \
		$(OBJS) \
		-Wl,--whole-archive \
		$(PLUGIN_LIBS) \
		$(PLUGIN_COMPILER_LIBS) \
		-Wl,--no-whole-archive \
		-Wl,--start-group \
		$(ADAPTER_LIBS) \
		$(CORE_LIBS) \
		-Wl,--end-group \
		$(LDLIBS) \
		-o $@


$(BUILD_DIR)/$(NAME)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/$(NAME)
	rm -f $(TARGET)