target_sources(
    ${TARGET}
    PRIVATE
        examples/examples.qrc
        icons/icons.qrc
)

set_target_properties(
    ${TARGET}
    PROPERTIES
        AUTORCC ON
)
