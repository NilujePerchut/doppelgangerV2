TOP_DIR := $(CURDIR)/..
BUILD_DIR := $(CURDIR)/build
SRCS_DIR := $(CURDIR)/src
UNITS_DIR := $(CURDIR)/units
UTILS_DIR := $(TOP_DIR)/utils

# KICAD libs
KICAD_LIBS := "/usr/share/kicad/library"
KICAD_MODS := /usr/share/kicad/modules
KICAD_PRJ_LIBS := $(CURDIR)/../lib

# Units tests
UNITS_SRC := $(wildcard $(UNITS_DIR)/*.py)
UNITS_NETS := $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.py,%.net,$(UNITS_SRC))))
UNITS_PCBS := $(patsubst %.net,%.kicad_pcb,$(UNITS_NETS))

PYTHONPATH := $(TOP_DIR):$(CURDIR)

# Preserves netlists
.PRECIOUS: $(UNITS_NETS)

# Global targets
UNITS := $(UNITS_PCBS)
MAIN_BOARD := $(BUILD_DIR)/main_board.net

all: $(UNITS) $(MAIN_BOARD)


$(BUILD_DIR)/%.kicad_pcb: $(BUILD_DIR)/%.net
	@# SKIDL is generating log, erc and net files in the
	@# top-level dir. So launch it from the build dir
	( export KIPRJMOD=$(KICAD_PRJ_LIBS) && \
	  export KISYSMOD=$(KICAD_MODS) && \
	  python $(UTILS_DIR)/kinet2pcb.py -d7 -w -nb --input $<  --output $@ \
	)


# Units py -> net
$(BUILD_DIR)/%.net: $(UNITS_DIR)/%.py | $(BUILD_DIR)
	@echo building $@ from $<
	@# SKIDL is generating log, erc and net files in the
	@# top-level dir. So launch it from the build dir
	( export KICAD_SYMBOL_DIR=$(KICAD_LIBS) && \
	  export KIPRJLIB=$(KICAD_PRJ_LIBS) && \
	  export PYTHONPATH=${PYTHONPATH} && \
	  cd $(BUILD_DIR) && \
	  $< $@ \
	)

# Main Board py -> net
$(BUILD_DIR)/%.net: $(SRCS_DIR)/%.py | $(BUILD_DIR)
	@echo building $@ from $<
	@# SKIDL is generating log, erc and net files in the
	@# top-level dir. So launch it from the build dir
	( export KICAD_SYMBOL_DIR=$(KICAD_LIBS) && \
	  export KIPRJLIB=$(KICAD_PRJ_LIBS) && \
	  export PYTHONPATH=${PYTHONPATH} && \
	  cd $(BUILD_DIR) && \
	  $< $@ \
	)

$(BUILD_DIR):
	mkdir -p $@


clean:
	rm -rf $(BUILD_DIR)
