SET(TESTS
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
		${TESTS}
)

target_link_libraries(wire_system-tests
	PRIVATE
		wire_system
		doctest::doctest
)
