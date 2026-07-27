set(TARGET qschematic-tests)

add_executable(${TARGET})

target_sources(
    ${TARGET}
    PRIVATE
        test_main.cpp
        tests/scene.cpp
        tests/scene_restore.cpp
)

target_compile_features(
    ${TARGET}
    PUBLIC
        cxx_std_23
)

# Reuse the doctest header vendored for the wire system tests
target_include_directories(
    ${TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../wire_system/test/3rdparty
)

target_link_libraries(
    ${TARGET}
    PUBLIC
        ${QSCHEMATIC_TARGET_INTERNAL}
)

add_test(NAME ${TARGET} COMMAND ${TARGET})
