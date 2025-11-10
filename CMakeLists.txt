cmake_minimum_required(VERSION 3.16)

project(CGG_PW_3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Используем необходимые компоненты для OpenGL вместо Quick
find_package(Qt6 REQUIRED COMPONENTS Core OpenGL Widgets)

# Ищем GLM
find_package(glm REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

# Для macOS: настройки OpenGL
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

# Добавляем все необходимые файлы
qt_add_executable(appCGG_PW_3
    main.cpp
    OpenGLWindow.cpp
    OpenGLWindow.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCGG_PW_3 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCGG_PW_3
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Подключаем необходимые библиотеки для OpenGL
target_link_libraries(appCGG_PW_3
    PRIVATE
        Qt6::Core
        Qt6::OpenGL
        Qt6::Widgets
        glm::glm
)

# Для macOS: линкуем системную библиотеку OpenGL
if(APPLE)
    target_link_libraries(appCGG_PW_3 PRIVATE ${OPENGL_LIBRARY})
endif()

include(GNUInstallDirs)
install(TARGETS appCGG_PW_3
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
