add_executable(minimal main.cpp)

target_compile_features(minimal PRIVATE cxx_std_17)


target_include_directories(minimal PRIVATE
    $ENV{CPATH}
    ${CMAKE_CURRENT_BINARY_DIR}/
)

target_link_libraries(minimal PRIVATE
    embree3
)

set_target_properties(minimal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)