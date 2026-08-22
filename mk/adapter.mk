include $(PROJECT_ROOT)/mk/common.mk


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
	$(CC) $(CPPFLAGS) $(CFLAGS_COMMON) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR)/$(NAME)
	rm -f $(TARGET)

COMPONENT_INFO := $(BUILD_DIR)/lib/adapters/$(PLATFORM)/$(NAME)/component.mk

$(COMPONENT_INFO):
	@mkdir -p $(dir $@)
	@echo "AP_ADAPTER_LIBS += $(TARGET)" > $@
	@echo "AP_ADAPTER_LDLIBS += $(LDLIBS)" >> $@

all: $(COMPONENT_INFO)