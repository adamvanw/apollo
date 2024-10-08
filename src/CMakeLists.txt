add_executable(${PROJECT_NAME}
        action.h
        draw.h
        general.h
        qoi.h
        oneeuro.h
        includes.h
        fitcurve.h
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imconfig.h
        imgui/backends/imgui_impl_sdlrenderer3.cpp
        imgui/backends/imgui_impl_sdlrenderer3.h
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdl3.h
)
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)