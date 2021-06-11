include(FetchContent)

if (QSCHEMATIC_USE_GPDS)
# GPDS
    if (QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD)
      FetchContent_Declare(
         gpds
         GIT_REPOSITORY https://github.com/simulton/gpds
            GIT_TAG        1.0.2
        )
     FetchContent_GetProperties(gpds)
      if(NOT gpds_POPULATED)
            FetchContent_Populate(gpds)
           set(GPDS_BUILD_TESTS OFF CACHE INTERNAL "")
          set(GPDS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
           add_subdirectory(${gpds_SOURCE_DIR} ${gpds_BINARY_DIR})
        endif()
    endif()
endif()

# Qt5
find_package(
    Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
)
