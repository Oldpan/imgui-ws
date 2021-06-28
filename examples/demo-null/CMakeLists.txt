
find_package(OpenCV REQUIRED)

add_executable(demo-null main.cpp)
target_include_directories(demo-null PRIVATE 
                            ..
                            ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(demo-null PRIVATE 
                    imgui 
                    imgui-ws
                    ${OpenCV_LIBS})
