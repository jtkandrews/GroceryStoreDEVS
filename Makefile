# ------------------------
# Compiler settings
# ------------------------
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# ------------------------
# Paths
# ------------------------
CADMIUM_INCLUDE ?= CADMIUM_INCLUDE ?= $(addsuffix /include,\
    $(shell find $(HOME) -maxdepth 6 -type d -name "cadmium_v2" 2>/dev/null | head -1))

ATOMICS_DIR   = atomics
TOPMODEL_DIR  = top_model
COUPLED_DIR   = coupled
TESTS_DIR     = test
BIN_DIR       = bin

INCLUDES = -I$(CADMIUM_INCLUDE) -I$(ATOMICS_DIR) -I$(TOPMODEL_DIR) -I$(COUPLED_DIR) -I$(TESTS_DIR)

# ------------------------
# Main sim
# ------------------------
MAIN_SRC = $(TOPMODEL_DIR)/main.cpp

# Rebuild when headers change
HEADERS = $(wildcard $(ATOMICS_DIR)/*.hpp) $(wildcard $(ATOMICS_DIR)/*.h) \
          $(wildcard $(TOPMODEL_DIR)/*.hpp) $(wildcard $(TOPMODEL_DIR)/*.h) \
          $(wildcard $(TESTS_DIR)/*.hpp)   $(wildcard $(TESTS_DIR)/*.h)

# ------------------------
# Tests (auto-discover)
# Any tests/foo.cpp -> bin/foo
# ------------------------
TEST_SRCS  = $(wildcard $(TESTS_DIR)/*.cpp)
TEST_BINS  = $(patsubst $(TESTS_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRCS))

# ------------------------
# Default target
# ------------------------
all: grocery_sim

# ------------------------
# Build grocery simulation executable
# ------------------------
grocery_sim: $(MAIN_SRC) $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/grocery_sim $(MAIN_SRC)

run: grocery_sim
	./$(BIN_DIR)/grocery_sim

# ------------------------
# Build all tests
# ------------------------
tests: $(TEST_BINS)

# Pattern rule: build each test binary from its .cpp
$(BIN_DIR)/%: $(TESTS_DIR)/%.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

# Run all tests (just executes each test binary)
run-tests: tests
	@set -e; \
	for t in $(TEST_BINS); do \
		echo "---- Running $$t ----"; \
		./$$t; \
	done

# Convenience: run a single test by name:
#   make run-test_one_customer
# (works for any tests/<name>.cpp)
run-%: $(BIN_DIR)/%
	./$(BIN_DIR)/$*

# ------------------------
# Create bin directory
# ------------------------
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ------------------------
# Clean build artifacts
# ------------------------
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean run tests run-tests
