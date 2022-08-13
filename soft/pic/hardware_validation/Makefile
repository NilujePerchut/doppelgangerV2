BUILD_DIR := $(CURDIR)/build
SRC_DIR := $(CURDIR)/src
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(subst .c,.o,$(SRCS))))

TARGET := $(BUILD_DIR)/hardware_validation.hex

ARCH := pic14
DEVICE := 16f18855
CC := sdcc
CFLAGS := --use-non-free -m$(ARCH) -p$(DEVICE)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

# The main.o should be given first to sdcc (according to the user manual)

$(TARGET): $(BUILD_DIR)/main.o $(OBJS) | $(BUILD_DIR)
	(cd $(BUILD_DIR) && $(CC) -o $@ $(CFLAGS) $^)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	(cd $(BUILD_DIR) && $(CC) $(CFLAGS) -c $<)

$(BUILD_DIR)/main.o: $(CURDIR)/main.c | $(BUILD_DIR)
	(cd $(BUILD_DIR) && $(CC) $(CFLAGS) -c $<)

$(BUILD_DIR):
	mkdir -p $@
