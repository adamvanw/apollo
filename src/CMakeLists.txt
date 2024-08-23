add_executable(${PROJECT_NAME}
        action.h
        draw.h
        general.h
        qoi.h
        oneeuro.h
        includes.h
        fitcurve.h)
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)