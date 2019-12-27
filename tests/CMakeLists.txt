add_executable(tests
    main.cpp
)

target_compile_features(tests PRIVATE cxx_std_17)

if(WIN32)
    target_compile_options(tests PRIVATE /WX)
elseif(UNIX)
    target_compile_options(tests PRIVATE -Wall -Werror)
endif(WIN32)

target_include_directories(tests PRIVATE
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(tests PRIVATE 
    yecs-include
)
