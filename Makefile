# Compiler settings
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# --------- Paths (EDIT THESE) ----------
# Put cadmium_v2 and your repo side-by-side in WSL, e.g.:
#   ~/cadmium_v2
#   ~/GROCERYSTOREDEVS
#
# Then CADMIUM_INCLUDE can be:
#   ../cadmium_v2/include
CADMIUM_INCLUDE ?= $(addsuffix /include,\
    $(shell find $(HOME) -maxdepth 6 -type d -name "cadmium_v2" 2>/dev/null | head -1))


# Validate it was actually found
ifeq ($(CADMIUM_INCLUDE),)
$(error CADMIUM_INCLUDE not found. Run: make CADMIUM_INCLUDE=/path/to/cadmium_v2/include)
endif

# Your project structure
ATOMICS_DIR   = atomics
TOPMODEL_DIR  = top_model

INCLUDES = -I$(CADMIUM_INCLUDE) -I$(ATOMICS_DIR) -I$(TOPMODEL_DIR)

# Output directory
BIN_DIR = bin

# Source files
MAIN_SRC = $(TOPMODEL_DIR)/main.cpp

# Grab headers so 'make' rebuilds when you change them
HEADERS = $(wildcard $(ATOMICS_DIR)/*.hpp) $(wildcard $(ATOMICS_DIR)/*.h) \
          $(wildcard $(TOPMODEL_DIR)/*.hpp) $(wildcard $(TOPMODEL_DIR)/*.h)

# Default target
all: test_generator grocery_sim

# Build test_generator
test_generator: test/test_generator.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_generator test/test_generator.cpp 

# Build grocery simulation executable
grocery_sim: $(MAIN_SRC) $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/grocery_sim $(MAIN_SRC)

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)



# Run target
run: grocery_sim
	./$(BIN_DIR)/grocery_sim

run_generator: test_generator
	./$(BIN_DIR)/test_generator

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean run