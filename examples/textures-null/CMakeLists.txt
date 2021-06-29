find_package(OpenCV REQUIRED)
add_executable(textures-null main.cpp)
target_include_directories(textures-null PRIVATE ${OpenCV_INCLUDE_DIRS} ..)
target_link_libraries(textures-null PRIVATE imgui imgui-ws ${OpenCV_LIBS})
