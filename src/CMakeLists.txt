add_executable(${PROJECT_NAME}
        action.h
        draw.h
        general.h
        qoi.h
        oneeuro.h
        includes.h
        fitcurve.h
        imgui/imgui.h
        imgui/backends/imgui_impl_sdl3.h
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdlrenderer3.h
        imgui/backends/imgui_impl_sdlrenderer3.cpp
        imgui/imconfig.h
        imgui/imgui_internal.h
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
)
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)