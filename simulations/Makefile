CXX := g++
CXXFLAGS := -O3 -Iinclude
BIN_DIR := bin
DATA_DIRS := data-ordering-wp data-ordering-aws data-ordering-gossip \
             data-unspool-wp data-unspool-aws data-unspool-gossip \
			 data-found_templates
EXECUTABLES := $(BIN_DIR)/search_template $(BIN_DIR)/unspool

.PHONY: all dir clean

all: $(EXECUTABLES)

dir:
	mkdir -p $(BIN_DIR) $(DATA_DIRS)

$(EXECUTABLES): | dir

$(BIN_DIR)/%: src-unspooling_algo/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

run:
	chmod +x run_exp-wp.sh run_exp-aws.sh run_exp-gossip.sh

clean:
	rm -rf $(BIN_DIR) $(DATA_DIRS)
