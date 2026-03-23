cmake_minimum_required(VERSION 3.14)

project(GroceryStoreDEVS VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Cadmium is expected as a sibling folder next to this project: ../cadmium_v2/include
set(CADMIUM_INCLUDE ${CMAKE_SOURCE_DIR}/../cadmium_v2/include)

include_directories(${CADMIUM_INCLUDE} ${CMAKE_SOURCE_DIR}/atomics ${CMAKE_SOURCE_DIR}/coupled)

# Output all binaries to the project-level bin/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Header dependencies (for IDE support)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/atomics/*.hpp ${CMAKE_SOURCE_DIR}/coupled/*.hpp)

# Main simulation
add_executable(grocery_sim ${CMAKE_SOURCE_DIR}/top_model/main.cpp ${HEADERS})

# Atomic tests
add_executable(test_cash           ${CMAKE_SOURCE_DIR}/test/test_cash.cpp           ${HEADERS})
add_executable(test_payment        ${CMAKE_SOURCE_DIR}/test/test_payment.cpp        ${HEADERS})
add_executable(test_traveler       ${CMAKE_SOURCE_DIR}/test/test_traveler.cpp       ${HEADERS})
add_executable(test_distributor    ${CMAKE_SOURCE_DIR}/test/test_distributor.cpp    ${HEADERS})
add_executable(test_packer         ${CMAKE_SOURCE_DIR}/test/test_packer.cpp         ${HEADERS})
add_executable(test_curbside       ${CMAKE_SOURCE_DIR}/test/test_curbside.cpp       ${HEADERS})
add_executable(test_customer_sink  ${CMAKE_SOURCE_DIR}/test/test_customer_sink.cpp  ${HEADERS})
add_executable(test_generator      ${CMAKE_SOURCE_DIR}/test/test_generator.cpp      ${HEADERS})

# Integration tests
add_executable(test_one_customer   ${CMAKE_SOURCE_DIR}/test/test_one_customer.cpp   ${HEADERS})
add_executable(test_pickup_system  ${CMAKE_SOURCE_DIR}/test/test_pickup_system.cpp  ${HEADERS})
add_executable(test_full_system    ${CMAKE_SOURCE_DIR}/test/test_full_system.cpp    ${HEADERS})
