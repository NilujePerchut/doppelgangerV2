
BUILD_DIR := $(CURDIR)/../build/tests
DOP_SRC_DIR := $(CURDIR)/../src
DOP_SRCS := $(wildcard $(DOP_SRC_DIR)/*.c)
DOP_SRCS := $(filter-out $(DOP_SRC_DIR)/main.c,$(DOP_SRCS))
TESTS := $(wildcard $(CURDIR)/*.c)
WRAP_EXTRACTOR := $(CURDIR)/../../../tools/wrap_extractor.py

CFLAGS := --coverage -DTEST
INCLUDES := -I$(DOP_SRC_DIR) -I$(CURDIR)/fake_includes
LIBS := -lcmocka

.PHONY := test_keys_run

all: runs

cov: runs
	(cd $(BUILD_DIR) && gcovr -r ../../ -e $(CURDIR) --html-details coverage.html)
	(cd $(BUILD_DIR) && firefox coverage.html)

runs: test_keys_run

%_run: $(BUILD_DIR)/%
	$<

$(BUILD_DIR)/%: $(CURDIR)/%.c $(DOP_SRCS) | $(BUILD_DIR)
	$(eval WRAP := $(shell $(WRAP_EXTRACTOR) $<))
	gcc $(CFLAGS) $(INCLUDES) $(WRAP) $^ -o $@ $(LIBS)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

