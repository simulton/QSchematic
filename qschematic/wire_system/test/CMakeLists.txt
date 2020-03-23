###
# Testing (using doctest)
###

# Add doctest
configure_file(CMakeLists.txt.in doctest/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doctest )
if (result)
	message(FATAL_ERROR "CMake step for doctest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doctest )
if (result)
	message(FATAL_ERROR "Build step for doctest failed: ${result}")
endif()

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/doctest-src
		         ${CMAKE_CURRENT_BINARY_DIR}/doctest-build
			     EXCLUDE_FROM_ALL)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/doctest-src/doctest)

# Find tests
SET(TESTS
	tests/manager.cpp
	tests/nets.cpp
	tests/wire.cpp
	tests/line.cpp
)

# Build test application
add_executable(wire_system-tests
	test_main.cpp
	connector.h
	${TESTS})

target_link_libraries(wire_system-tests wire_system)

# Automatically discover tests
include(CTest)
include(${CMAKE_CURRENT_BINARY_DIR}/doctest-src/scripts/cmake/doctest.cmake)
doctest_discover_tests(wire_system-tests)
