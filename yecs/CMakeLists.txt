add_library(yecs-lib STATIC
    common.h
    component_storage.h
    component_types_builder.h
    entity_set.h
    entity_query.h
    entity_query.cc
    system.h
    world.h
    world.cc
    yecs.h
)

target_compile_features(yecs-lib PRIVATE cxx_std_17)

target_include_directories(yecs-lib PUBLIC ${PROJECT_SOURCE_DIR})

if(WIN32)
    target_compile_options(yecs-lib PRIVATE /WX)
elseif(UNIX)
    target_compile_options(yecs-lib PRIVATE -Wall -Werror)
endif(WIN32)


