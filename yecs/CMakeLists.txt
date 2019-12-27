add_library(yecs-include INTERFACE)

target_compile_features(yecs-include INTERFACE cxx_std_17)

set(YECS_INCLUDE_SOURCES
    common.h
    component_collection.h
    system.h
    world.h
    yecs.h
)

# trick to show files in the dedicated Visual Studio project
if(MSVC)
    add_custom_target(yecs-include-sources SOURCES
        ${YECS_INCLUDE_SOURCES}
    )
endif()

