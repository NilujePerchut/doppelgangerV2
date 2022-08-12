BUILD_DIR = $(CURDIR)/build
SRC_DIR = $(CURDIR)/src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(subst .c,.o,$(SRCS))))

TARGET = $(BUILD_DIR)/hardware_validation.hex

ARCH = pic14
DEVICE = 16f18855
CC = sdcc
CFLAGS = --use-non-free -m$(ARCH) -p$(DEVICE)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET): $(SRCS) | $(BUILD_DIR)
	(cd $(BUILD_DIR) && $(CC) $(CFLAGS) $^)

$(BUILD_DIR):
	mkdir -p $@
