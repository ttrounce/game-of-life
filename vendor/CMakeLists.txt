cmake_minimum_required (VERSION 3.6)

add_subdirectory(glfw)
add_subdirectory(glad)


file(WRITE vendor.cpp "")
add_library(cgl_vendor STATIC vendor.cpp "stb/stb_image.h")
target_link_libraries(cgl_vendor glfw glad)
target_include_directories(cgl_vendor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)

set_target_properties(cgl_vendor PROPERTIES OUTPUT_NAME bsvend)
