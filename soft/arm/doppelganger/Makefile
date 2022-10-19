BUILD_DIR := $(CURDIR)/build
SRC_DIR := $(CURDIR)/src
TEST_DIR := $(CURDIR)/test
LIBOPENCM3_DIR := $(CURDIR)/../lib/libopencm3
LIBOPENCM3_INCLUDE_DIR := $(LIBOPENCM3_DIR)/include
LIBOPENCM3_LIB_DIR := $(LIBOPENCM3_DIR)/lib
DBG_TTY ?= /dev/ttyACM0

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(subst .c,.o,$(SRCS))))
BINARY := $(BUILD_DIR)/doppelganger
LNK  := $(SRC_DIR)/doppelganger_bootloader.ld

PREFIX := arm-none-eabi-
FP_FLAGS := -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS := -mthumb -mcpu=cortex-m4 $(FP_FLAGS)
COMMON_CFLAGS := -ggdb3 $(ARCH_FLAGS)
CFLAGS := $(COMMON_CFLAGS) -Os -ffunction-sections -fdata-sections

# specs option MUST apear first
LDFLAGS := --specs=nosys.specs $(COMMON_CFLAGS) --static -nostartfiles -Wl,--gc-sections -Xlinker -Map=$(BUILD_DIR)/output.map
DBG_FLAGS := -ex 'target extended-remote $(DBG_TTY)'\
			 -ex 'monitor version'\
			 -ex 'monitor swdp_scan'\
			 -ex 'attach 1'\
			 -ex 'load'\
			 -ex 'compare-sections'\
			 -ex 'echo Loading complet\n'

all: $(BINARY).uf2

debug: $(BINARY).elf
	$(PREFIX)gdb $(DBG_FLAGS) $<

size: $(BINARY).elf
	$(PREFIX)size $^

%.uf2: %.bin
	$(SRC_DIR)/uf2conv_python3.py -c -f STM32F4 -b 0x08010000 -o "$@" "$<"

.SECONDARY: $(BINARY).bin

%.bin: %.elf
	$(PREFIX)objcopy -Obinary $< $@

$(BINARY).elf: $(OBJS) $(LNK) | $(BUILD_DIR)
	$(PREFIX)gcc $(LDFLAGS) -T$(LNK) -L$(LIBOPENCM3_LIB_DIR) $(OBJS) -o $@ -lopencm3_stm32f4

$(BUILD_DIR)/%.o:$(SRC_DIR)/%.c | $(BUILD_DIR)
	$(PREFIX)gcc $(CFLAGS) -DSTM32F4 -I$(LIBOPENCM3_INCLUDE_DIR) -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $@

test: | $(BUILD_DIR)
	make -C $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	make -C $(LIBOPENCM3_DIR) clean
