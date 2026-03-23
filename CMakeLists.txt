cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
	message(STATUS "Building with ESP32")
	include($ENV{IDF_PATH}/tools/cmake/project.cmake)
	project(GroceryStoreDEVS)

else()
	message(STATUS "Building without ESP32")
	project(GroceryStoreDEVS)

	# Route all compiled executables to the bin directory
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

	# Cadmium Path
	if(DEFINED ENV{CADMIUM})
		# Check both possibilities: CADMIUM points to root OR directly to include
		set(CADMIUM_PATHS $ENV{CADMIUM} $ENV{CADMIUM}/include)
	else()
		# Fallback: Cadmium as sibling folder to project
		set(CADMIUM_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../cadmium_v2/include)
	endif()

	# executable targets
	add_executable(grocery_sim       top_model/main.cpp)
	add_executable(test_cash         test/test_cash.cpp)
	add_executable(test_payment      test/test_payment.cpp)
	add_executable(test_traveler     test/test_traveler.cpp)
	add_executable(test_distributor  test/test_distributor.cpp)
	add_executable(test_packer       test/test_packer.cpp)
	add_executable(test_curbside     test/test_curbside.cpp)
	add_executable(test_customer_sink test/test_customer_sink.cpp)
	add_executable(test_generator    test/test_generator.cpp)
	add_executable(test_one_customer test/test_one_customer.cpp)
	add_executable(test_pickup_system test/test_pickup_system.cpp)
	add_executable(test_full_system  test/test_full_system.cpp)

	# Apply include directories and compiler flags to all targets
	set(TARGETS
		grocery_sim
		test_cash
		test_payment
		test_traveler
		test_distributor
		test_packer
		test_curbside
		test_customer_sink
		test_generator
		test_one_customer
		test_pickup_system
		test_full_system
	)

	foreach(TARGET ${TARGETS})
		target_include_directories(${TARGET} PRIVATE "." "atomics" "coupled" ${CADMIUM_PATHS})
		target_compile_options(${TARGET} PUBLIC -std=gnu++17)
	endforeach()
endif()
