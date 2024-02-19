###
# NOTE: This is a very ugly solution. Ideally the wire_system would be built into an object library and then both the
#       consuming QSchematic library as well as this test suite would consume the object library. However this becomes
#       a pain to manage when installing QSchematic as static library.
#       Maybe we'll find a better way to handle this in the future.
###

set(WIRESYSTEM_SOURCES
	../connectable.hpp
	../line.cpp
	../line.hpp
	../manager.cpp
	../manager.hpp
	../net.cpp
	../net.hpp
	../point.cpp
	../point.hpp
	../wire.cpp
	../wire.hpp
	../../utils.cpp
	../../utils.hpp
	../../settings.cpp
	../../settings.hpp
)

set(TESTS
	tests/manager.cpp
	tests/nets.cpp
	tests/wire.cpp
	tests/line.cpp
)

set(TARGET qschematic-wiresystem-tests)

add_executable(${TARGET})

target_sources(
	${TARGET}
	PRIVATE
		3rdparty/doctest.h
		test_main.cpp
		connector.hpp
		${WIRESYSTEM_SOURCES}
		${TESTS}
)

target_compile_features(
	${TARGET}
	PUBLIC
		cxx_std_17
)

target_link_libraries(
	${TARGET}
	PUBLIC
		${QSCHEMATIC_TARGET_INTERNAL}
)
