cmake_minimum_required(VERSION 3.16)
project(application_repo VERSION 0.0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

# Aktiviere Tests für das Projekt
enable_testing()


add_subdirectory(test)

find_package(Vulkan)
find_package(glfw3)

if(Vulkan_FOUND AND glfw3_FOUND)
    message(STATUS "Vulkan und GLFW found – activate Grafik-Support.")
    set(IMGUI_DIR third_party/imgui-master/)
    include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

    add_executable(Playground Debugging.cpp 
                            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
                            ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
                            ${IMGUI_DIR}/imgui.cpp 
                            ${IMGUI_DIR}/imgui_draw.cpp 
                            ${IMGUI_DIR}/imgui_demo.cpp 
                            ${IMGUI_DIR}/imgui_tables.cpp 
                            ${IMGUI_DIR}/imgui_widgets.cpp)

    # Include-Pfade für ImGui und die externen Libs bekannt machen
    target_include_directories(Playground PRIVATE
        third_party/imgui-master
        third_party/imgui-master/backends
        ${Vulkan_INCLUDE_DIRS}
    )

    set(LIBRARIES "glfw;Vulkan::Vulkan")
    target_link_libraries(Playground ${LIBRARIES})
else()
    message(STATUS "Debugging Live Tool not builded")
endif()