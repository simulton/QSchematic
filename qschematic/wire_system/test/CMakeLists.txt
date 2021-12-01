###
# NOTE: This is a very ugly solution. Ideally the wire_system would be built into an object library and then both the
#       consuming QSchematic library as well as this test suite would consume the object library. However this becomes
#       a pain to manage when installing QSchematic as static library.
#       Maybe we'll find a better way to handle this in the future.
###

# Include Qt5 packages
find_package(Qt5
	REQUIRED
	COMPONENTS
		Core
		Gui
)

set(WIRESYSTEM_SOURCES
	../connectable.h
	../line.cpp
	../line.h
	../manager.cpp
	../manager.h
	../net.cpp
	../net.h
	../point.cpp
	../point.h
	../wire.cpp
	../wire.h
	../../utils.cpp
	../../utils.h
	../../settings.cpp
	../../settings.h
)

set(TESTS
	tests/manager.cpp
	tests/nets.cpp
	tests/wire.cpp
	tests/line.cpp
)

add_executable(wire_system-tests)

target_sources(
	wire_system-tests
	PRIVATE
		3rdparty/doctest.h
		test_main.cpp
		connector.h
		${WIRESYSTEM_SOURCES}
		${TESTS}
)

target_compile_features(wire_system-tests
	PUBLIC
		cxx_std_17
)

set_target_properties(
	wire_system-tests
    PROPERTIES
		AUTOMOC ON
)

target_link_libraries(
	wire_system-tests
	PUBLIC
		Qt5::Core
		Qt5::Gui
		qschematic-static
)
