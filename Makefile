# Compiler settings
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Include paths
# Cadmium is expected as a sibling folder next to this project: ../cadmium_v2/include, update it to your actual path if different
CADMIUM_INCLUDE = ../cadmium_v2/include
INCLUDES        = -I$(CADMIUM_INCLUDE) -Iatomics -Icoupled

# Directories
BIN_DIR = bin

# Header dependencies
HEADERS = $(wildcard atomics/*.hpp) $(wildcard coupled/*.hpp)

# Default target: build everything
all: grocery_sim test_cash test_payment test_traveler test_distributor \
     test_packer test_curbside test_customer_sink test_generator \
     test_one_customer test_pickup_system test_full_system

# Main simulation
grocery_sim: top_model/main.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/grocery_sim top_model/main.cpp

# Atomic tests
test_cash: test/test_cash.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_cash test/test_cash.cpp

test_payment: test/test_payment.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_payment test/test_payment.cpp

test_traveler: test/test_traveler.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_traveler test/test_traveler.cpp

test_distributor: test/test_distributor.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_distributor test/test_distributor.cpp

test_packer: test/test_packer.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_packer test/test_packer.cpp

test_curbside: test/test_curbside.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_curbside test/test_curbside.cpp

test_customer_sink: test/test_customer_sink.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_customer_sink test/test_customer_sink.cpp

test_generator: test/test_generator.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_generator test/test_generator.cpp

# Integration tests
test_one_customer: test/test_one_customer.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_one_customer test/test_one_customer.cpp

test_pickup_system: test/test_pickup_system.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_pickup_system test/test_pickup_system.cpp

test_full_system: test/test_full_system.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BIN_DIR)/test_full_system test/test_full_system.cpp

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
	ln -sfn ../input_data $(BIN_DIR)/input_data

# Run targets
run_grocery_sim: grocery_sim
	./$(BIN_DIR)/grocery_sim

run_test_cash: test_cash
	./$(BIN_DIR)/test_cash

run_test_payment: test_payment
	./$(BIN_DIR)/test_payment

run_test_traveler: test_traveler
	./$(BIN_DIR)/test_traveler

run_test_distributor: test_distributor
	./$(BIN_DIR)/test_distributor

run_test_packer: test_packer
	./$(BIN_DIR)/test_packer

run_test_curbside: test_curbside
	./$(BIN_DIR)/test_curbside

run_test_customer_sink: test_customer_sink
	./$(BIN_DIR)/test_customer_sink

run_test_generator: test_generator
	./$(BIN_DIR)/test_generator

run_test_one_customer: test_one_customer
	./$(BIN_DIR)/test_one_customer

run_test_pickup_system: test_pickup_system
	./$(BIN_DIR)/test_pickup_system

run_test_full_system: test_full_system
	./$(BIN_DIR)/test_full_system

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean grocery_sim \
        test_cash test_payment test_traveler test_distributor test_packer \
        test_curbside test_customer_sink test_generator test_one_customer \
        test_pickup_system test_full_system \
        run_grocery_sim run_test_cash run_test_payment run_test_traveler \
        run_test_distributor run_test_packer run_test_curbside run_test_customer_sink \
        run_test_generator run_test_one_customer \
        run_test_pickup_system run_test_full_system


