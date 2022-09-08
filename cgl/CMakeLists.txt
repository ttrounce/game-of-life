link_libraries (cgl_vendor)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable (cgl_exec ${SOURCES})
target_include_directories (cgl_exec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cgl_exec PROPERTIES
    OUTPUT_NAME cgl
    RUNTIME_OUTPUT_DIRECTORY    ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin
)