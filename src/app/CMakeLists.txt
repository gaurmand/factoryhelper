set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(factoryhelper
    main.cpp
    mainwindow.cpp
)

target_link_libraries(factoryhelper PRIVATE "factorymodel" Qt6::Core Qt6::Widgets)
