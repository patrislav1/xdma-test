cmake_minimum_required(VERSION 3.5)

FIND_PACKAGE(ChimeraTK-DeviceAccess 01.02 REQUIRED)
include_directories(SYSTEM ${ChimeraTK-DeviceAccess_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS}")

# Get the name of all .cpp files in the directory
FILE(GLOB SOURCE_FILES *.cpp)

# Create one executable for each source file, with the same base name
foreach( SOURCE_FILE ${SOURCE_FILES} )
    # NAME_WE is the file name without ending
    get_filename_component(TARGET ${SOURCE_FILE} NAME_WE)
    add_executable(${TARGET} ${SOURCE_FILE})
    # Link the DeviceAccess library to the target executable
    target_link_libraries(${TARGET} ${ChimeraTK-DeviceAccess_LIBRARIES})
    # Set all required linker flags (incl. RPath so the libraries are always found)
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "${ChimeraTK-DeviceAccess_LINKER_FLAGS}")

    # We add the target as a test. Run them with 'make test'. In a real application you only do this with the tests.
    add_test(${TARGET} ${TARGET})

    # Install the executable when calling 'make install'
    install(TARGETS ${TARGET} DESTINATION bin)
endforeach( SOURCE_FILE )

# To run the examples we need the map files. Copy them to the build folder.
FILE(COPY example.dmap my_device.map DESTINATION ${PROJECT_BINARY_DIR})

#add_subdirectory ("custom_backend_registration")
