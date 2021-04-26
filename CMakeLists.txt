cmake_minimum_required(VERSION 3.5)

PROJECT("XDMA-Test")

FIND_PACKAGE(ChimeraTK-DeviceAccess 01.02 REQUIRED)
include_directories(SYSTEM ${ChimeraTK-DeviceAccess_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS}")

FILE(GLOB SOURCE_FILES *.cpp)
add_executable(xdma-test ${SOURCE_FILES})
target_link_libraries(xdma-test ${ChimeraTK-DeviceAccess_LIBRARIES})
set_target_properties(xdma-test PROPERTIES LINK_FLAGS "${ChimeraTK-DeviceAccess_LINKER_FLAGS}")

install(TARGETS xdma-test DESTINATION bin)

# To run the examples we need the map files. Copy them to the build folder.
FILE(COPY example.dmap lfsr_test.map DESTINATION ${PROJECT_BINARY_DIR})
